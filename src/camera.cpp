#include "camera.hpp"
#include "configurations.hpp"
#include "custom_shaders.hpp"
#include "game.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/map_object.hpp"
#include "sprite.hpp"
#include "utility/color.hpp"
#include "utility/file.hpp"
#include "utility/math.hpp"
#include "xd/graphics/shaders.hpp"
#include "xd/graphics/transform_geometry.hpp"
#include "xd/graphics/vertex_batch.hpp"
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <algorithm>
#include <stdexcept>
#include <cstdlib>

struct Camera::Impl {
    Impl(const std::string& default_scale_mode, xd::vec4 clear_color)
        : postprocessing_enabled(Configurations::get<bool>("graphics.postprocessing-enabled"))
        , default_clear_color(clear_color)
        , default_scale_mode(default_scale_mode) {}
    // Update OpenGL viewport
    void update_viewport(xd::rect viewport, xd::vec2 shake_offset = xd::vec2{0.0f}) const {
        glViewport(static_cast<int>(viewport.x + shake_offset.x),
            static_cast<int>(viewport.y + shake_offset.y),
            static_cast<int>(viewport.w),
            static_cast<int>(viewport.h));
    }
    // Full-screen shader data
    bool postprocessing_enabled;
    xd::sprite_batch full_screen_batch;
    std::shared_ptr<xd::texture> full_screen_texture;
    // Configured screen clearing color
    xd::vec4 default_clear_color;
    // Last screen size
    xd::ivec2 screen_size;
    // Default environment scale mode
    std::string default_scale_mode;
    // Apply a certain shader
    void set_shader(const std::string& vertex, const std::string& fragment);
    // Render a full-screen shader
    void render_shader(Game& game, const xd::rect& viewport, xd::transform_geometry& geometry,
        float brightness, float contrast, float saturation);
    // Update OpenGL claer color
    void set_gl_clear_color(const xd::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }
    // Setup OpenGL state
    void setup_opengl() {
        set_gl_clear_color(default_clear_color);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
    }
    // Draw a quad (for screen tint)
    struct quad_vertex {
        xd::vec2 pos;
    };
    struct quad_vertex_traits : xd::vertex_traits<quad_vertex> {
        quad_vertex_traits() {
            bind_vertex_attribute(xd::VERTEX_POSITION, &quad_vertex::pos);
        }
    };
    void draw_quad(xd::mat4 mvp, xd::rect rect,
        const xd::vec4& color, GLenum draw_mode = GL_QUADS);
};

void Camera::Impl::set_shader(const std::string& vertex, const std::string& fragment) {
    if (!postprocessing_enabled) return;

    std::string vsrc, fsrc;
    if (!vertex.empty() && !fragment.empty()) {
        try {
            auto fs = file_utilities::game_data_filesystem();
            vsrc = fs->read_file(vertex);
            fsrc = fs->read_file(fragment);
        } catch (const std::runtime_error& err) {
            throw std::runtime_error("Error loading shaders: " + std::string{err.what()});
        }
    }

    if (!vsrc.empty()) {
        full_screen_batch.set_shader(std::make_unique<Custom_Shader>(vsrc, fsrc));
    } else {
        full_screen_batch.set_shader(std::make_unique<xd::fullscreen_shader>());
    }
}

void Camera::Impl::render_shader(Game& game, const xd::rect& viewport, xd::transform_geometry& geometry,
        float brightness, float contrast, float saturation) {
    if (!postprocessing_enabled) return;

    const int w = game.framebuffer_width();
    const int h = game.framebuffer_height();
    // Width and height can be 0 in some debugging scenarios
    if (w == 0 || h == 0) return;

    if (!full_screen_texture) {
        full_screen_texture = std::make_shared<xd::texture>(w, h, nullptr,
            xd::vec4(0), GL_CLAMP, GL_CLAMP);
    }
    full_screen_texture->copy_read_buffer(0, 0, w, h);
    full_screen_batch.clear();
    full_screen_batch.add(full_screen_texture, 0, 0);

    geometry.projection().push(
    xd::ortho<float>(
            0, static_cast<float>(w), // left, right
            0, static_cast<float>(h), // bottom, top
            -1, 1 // near, far
        )
    );
    geometry.model_view().push(xd::mat4());

    xd::rect full_screen_viewport{0, 0, w, h};
    auto same_viewport = viewport == full_screen_viewport;
    if (!same_viewport) {
        update_viewport(full_screen_viewport);
    }

    full_screen_batch.draw(xd::shader_uniforms{geometry.mvp(), game.ticks(),
        brightness, contrast, saturation});

    if (!same_viewport) {
        update_viewport(viewport);
    }

    geometry.model_view().pop();
    geometry.projection().pop();
}

void Camera::Impl::draw_quad(xd::mat4 mvp, xd::rect rect,
    const xd::vec4& color, GLenum draw_mode) {
    static xd::flat_shader shader;
    // Setup uniforms
    mvp = xd::translate(mvp, xd::vec3{rect.x, rect.y, 0.0f});
    shader.setup(mvp, color);

    // Set quad vertex positions
    Camera::Impl::quad_vertex quad[4];
    quad[0].pos = xd::vec2(0.0f, rect.h);
    quad[1].pos = xd::vec2(rect.w, rect.h);
    quad[2].pos = xd::vec2(rect.w, 0.0f);
    quad[3].pos = xd::vec2(0.0f, 0.0f);

    // Load and render batch
    xd::vertex_batch<Camera::Impl::quad_vertex_traits> batch(draw_mode);
    batch.load(&quad[0], 4);
    batch.render();
}

Camera::Camera(Game& game, const std::string& default_scale_mode)
        : game(game)
        , position(0.0f, 0.0f)
        , screen_tint(hex_to_color(Configurations::get<std::string>("startup.tint-color")))
        , brightness(Configurations::get<float>("graphics.brightness"))
        , contrast(Configurations::get<float>("graphics.contrast"))
        , saturation(Configurations::get<float>("graphics.saturation"))
        , object(nullptr)
        , object_center_offset(Configurations::get<float>("player.camera-center-offset-x"),
            Configurations::get<float>("player.camera-center-offset-y"))
        , shaker(nullptr)
        , current_clear_color(hex_to_color(Configurations::get<std::string>("startup.clear-color")))
        , pimpl(std::make_unique<Impl>(default_scale_mode, current_clear_color)) {
    add_component(std::make_shared<Camera_Renderer>(game));
    add_component(std::make_shared<Object_Tracker>());

    pimpl->setup_opengl();
    const auto width = game.framebuffer_width();
    const auto height = game.framebuffer_height();
    set_viewport(calculate_viewport(width, height));
}

Camera::~Camera() {}

void Camera::set_size(int width, int height, bool force) {
    if (!force && width == pimpl->screen_size.x && height == pimpl->screen_size.y) return;
    LOGGER_I << "Setting camera size to " << width << "x" << height;
    pimpl->full_screen_texture.reset();
    set_viewport(calculate_viewport(width, height));
    pimpl->screen_size.x = width;
    pimpl->screen_size.y = height;
}

xd::rect Camera::calculate_viewport(int width, int height) {
    auto scale_mode = Configurations::get<std::string>("graphics.scale-mode");
    const auto screen_width = static_cast<float>(width);
    const auto screen_height = static_cast<float>(height);
    const auto game_width = static_cast<float>(game.game_width(false));
    const auto game_height = static_cast<float>(game.game_height(false));

    if (scale_mode == "default") {
        scale_mode = pimpl->default_scale_mode.empty()
            ? "aspect"
            : pimpl->default_scale_mode;
    }

    if (scale_mode == "stretch") {
        calculated_viewport.x = 0.0f;
        calculated_viewport.y = 0.0f;
        calculated_viewport.w = screen_width;
        calculated_viewport.h = screen_height;
        return calculated_viewport;
    }

    if (scale_mode == "none") {
        calculated_viewport.w = std::min(screen_width, game_width);
        calculated_viewport.h = std::min(screen_height, game_height);
    } else if (scale_mode == "aspect" ||
            screen_width < game_width || screen_height < game_height) {
        float target_ratio = game_width / game_height;

        calculated_viewport.w = screen_width;
        calculated_viewport.h = calculated_viewport.w / target_ratio;

        if (calculated_viewport.h > screen_height) {
            calculated_viewport.h = screen_height;
            calculated_viewport.w = calculated_viewport.h * target_ratio;
        }
    } else {
        int scale_x = static_cast<int>(screen_width / game_width);
        int scale_y = static_cast<int>(screen_height / game_height);
        if (scale_x > scale_y) {
            calculated_viewport.w = game_width * scale_y;
            calculated_viewport.h = game_height * scale_y;
        } else {
            calculated_viewport.w = game_width * scale_x;
            calculated_viewport.h = game_height * scale_x;
        }
    }

    calculated_viewport.x = screen_width / 2 - calculated_viewport.w / 2;
    calculated_viewport.y = screen_height / 2 - calculated_viewport.h / 2;

    return calculated_viewport;
}

void Camera::update_viewport(xd::vec2 shake_offset) const {
    pimpl->update_viewport(viewport, shake_offset);
}

void Camera::set_clear_color(std::optional<xd::vec4> color) {
    current_clear_color = color.value_or(pimpl->default_clear_color);
    pimpl->set_gl_clear_color(current_clear_color);
}

void Camera::clear_color_buffer() const {
    glClear(GL_COLOR_BUFFER_BIT);
}

void Camera::clear_depth_buffer() const {
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Camera::clear() const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Camera::center_at(xd::vec2 pos) {
    position = get_centered_position(pos);
}

void Camera::center_at(const Map_Object& target) {
    position = get_centered_position(target);
}

xd::vec2 Camera::get_centered_position(xd::vec2 pos) const {
    return get_bounded_position(pos - game.game_center());
}

xd::vec2 Camera::get_centered_position(const Map_Object& target) const {
    const auto sprite = target.get_sprite();
    const auto sprite_size = sprite ? sprite->get_size() : xd::vec2{0.0f, 0.0f};
    const auto object_pos = target.get_position() + sprite_size / 2.0f;
    return get_centered_position(object_pos + object_center_offset);
}

xd::rect Camera::get_position_bounds() const {
    const auto map = game.get_map();
    const float map_width = static_cast<float>(map->get_pixel_width());
    const float map_height = static_cast<float>(map->get_pixel_height());
    const float game_width = static_cast<float>(game.game_width());
    const float game_height = static_cast<float>(game.game_height());
    const float width_diff = map_width - game_width;
    const float height_diff = map_height - game_height;

    return xd::rect{0.0f, 0.0f, width_diff, height_diff};
}

xd::vec2 Camera::get_bounded_position(xd::vec2 pos) const {
    auto bounds = get_position_bounds();
    return xd::vec2{std::min(bounds.w, std::max(bounds.x, pos.x)),
                        std::min(bounds.h, std::max(bounds.y, pos.y))};
}

void Camera::set_position(xd::vec2 pos) {
    position = get_bounded_position(pos);
}

void Camera::draw_rect(xd::rect rect, xd::vec4 color, bool fill) const {
    GLenum draw_mode = fill ? GL_QUADS :  GL_LINE_LOOP;
    pimpl->draw_quad(geometry.mvp(), rect, color, draw_mode);
}

void Camera::enable_scissor_test(xd::rect rect, xd::rect custom_viewport) {
    if (custom_viewport.w == 0) {
        custom_viewport = viewport;
    }

    const float game_width = static_cast<float>(game.game_width());
    const float game_height = static_cast<float>(game.game_height());
    const int y = static_cast<int>(game_height - (rect.y + rect.h));
    const xd::vec2 scale{custom_viewport.w / game_width,
                         custom_viewport.h / game_height};

    glEnable(GL_SCISSOR_TEST);
    glScissor(static_cast<int>(custom_viewport.x + rect.x * scale.x),
             static_cast<int>(custom_viewport.y + y * scale.y),
             static_cast<int>(rect.w * scale.x),
             static_cast<int>(rect.h * scale.y));
}

void Camera::disable_scissor_test() {
    glDisable(GL_SCISSOR_TEST);
}

void Camera::set_shader(const std::string& vertex, const std::string& fragment) {
    pimpl->set_shader(vertex, fragment);
}

void Camera::render_shader() {
    pimpl->render_shader(game, viewport, geometry, brightness, contrast, saturation);
}

void Camera::start_shaking(xd::vec2 strength, xd::vec2 speed) {
    if (shaker) {
        shaker->update(*this);
        shaker->change_settings(strength, speed);
        return;
    }

    shaker = std::make_shared<Screen_Shaker>(strength, speed);
    add_component(shaker);
}

void Camera::cease_shaking() {
    del_component(shaker);
    shaker.reset();
    update_viewport();
}

xd::vec2 Camera::shake_offset() const {
    return is_shaking() ? shaker->shake_offset() : xd::vec2{0.0f};
}

void Camera::draw_map_tint() const {
    if (check_close(map_tint.a, 0.0f)) return;

    const auto width = static_cast<float>(game.game_width());
    const auto height = static_cast<float>(game.game_height());

    auto cam_pos = get_pixel_position();
    const xd::rect rect{cam_pos.x, cam_pos.y, width, height};
    draw_rect(rect, map_tint);
}

void Camera_Renderer::render(Camera& camera) {
    camera.clear();

    const auto width = static_cast<float>(game.game_width());
    const auto height = static_cast<float>(game.game_height());

    auto& geometry = camera.get_geometry();
    // left, right, bottom, top, near, far
    geometry.projection().load(xd::ortho<float>(0, width, height, 0, -1, 1));

    geometry.model_view().identity();
    const auto cam_pos = camera.get_pixel_position();
    geometry.model_view().translate(-cam_pos.x, -cam_pos.y, 0);

    if (camera.is_shaking()) {
        camera.set_shake_offset(camera.shake_offset());
    }

    game.get_map()->render();

    const auto tint_color = camera.get_screen_tint();
    if (!check_close(tint_color.a, 0.0f)) {
        const xd::rect rect{cam_pos.x, cam_pos.y, width, height};
        camera.draw_rect(rect, tint_color);
    }

    camera.render_shader();
}

void Object_Tracker::update(Camera& camera) {
    const auto obj = camera.get_object();
    if (!obj) return;

    camera.center_at(*obj);
}

Screen_Shaker::Screen_Shaker(xd::vec2 strength, xd::vec2 speed)
        : last_direction(1.0f, 1.0f)
        , offset(0.0f, 0.0f) {
    change_settings(strength, speed);
    // Randomize the starting direction
    const auto max = static_cast<double>(RAND_MAX);
    if (std::rand() / max < 0.5) last_direction.x = -1.0f;
    if (std::rand() / max < 0.5) last_direction.y = -1.0f;
}

void Screen_Shaker::change_settings(xd::vec2 new_strength, xd::vec2 new_speed) {
    strength = new_strength;
    const auto logic_fps = Configurations::get<int>("graphics.logic-fps", "debug.logic-fps");
    speed = new_speed * (60.0f / logic_fps);

    // Prevents changing direction twice if offset was already over the strength limit
    offset.x = std::max(-strength.x * 2, std::min(strength.x * 2, offset.x));
    offset.y = std::max(-strength.y * 2, std::min(strength.y * 2, offset.y));
}

void Screen_Shaker::update(Camera&) {
    offset += (strength * speed * last_direction) / 10.0f;
    if (offset.x > strength.x * 2 || offset.x < -strength.x * 2) {
        last_direction.x *= -1.0f;
    }
    if (offset.y > strength.y * 2 || offset.y < -strength.y * 2) {
        last_direction.y *= -1.0f;
    }
}
