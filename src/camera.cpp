#include "../include/camera.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/sprite.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/color.hpp"
#include "../include/utility/math.hpp"
#include "../include/configurations.hpp"
#include "../include/log.hpp"
#include "../include/custom_shaders.hpp"
#include "../include/xd/graphics/vertex_batch.hpp"
#include "../include/xd/graphics/shader_program.hpp"
#include "../include/xd/graphics/shaders.hpp"
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <algorithm>
#include <stdexcept>
#include <cstdlib>

struct Camera::Impl {
    explicit Impl()
        : postprocessing_enabled(Configurations::get<bool>("graphics.postprocessing-enabled"))
        , clear_color(hex_to_color(Configurations::get<std::string>("startup.clear-color"))) {}
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
    // Screen clearing color
    xd::vec4 clear_color;
    // Last screen size
    xd::ivec2 screen_size;
    // Apply a certain shader
    void set_shader(const std::string& vertex, const std::string& fragment);
    // Render a full-screen shader
    void render_shader(Game& game, const xd::rect& viewport, xd::transform_geometry& geometry, float brightness, float contrast);
    // Draw a quad (for screen tint)
    struct quad_vertex
    {
        xd::vec2 pos;
    };
    struct quad_vertex_traits : xd::vertex_traits<quad_vertex>
    {
        quad_vertex_traits()
        {
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

void Camera::Impl::render_shader(Game& game, const xd::rect& viewport, xd::transform_geometry& geometry, float brightness, float contrast) {
    if (!postprocessing_enabled) return;

    int w = game.framebuffer_width();
    int h = game.framebuffer_height();
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
    auto same_viewport = viewport == xd::rect{0, 0, w, h};
    if (!same_viewport)
        glViewport(0, 0, w, h);
    full_screen_batch.draw(xd::shader_uniforms{geometry.mvp(), game.ticks(), brightness, contrast});
    if (!same_viewport)
        update_viewport(viewport);
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

Camera::Camera(Game& game)
        : game(game),
        position(xd::vec2(0.0f, 0.0f)),
        screen_tint(hex_to_color(Configurations::get<std::string>("startup.tint-color"))),
        brightness(Configurations::get<float>("graphics.brightness")),
        contrast(Configurations::get<float>("graphics.contrast")),
        object(nullptr),
        shaker(nullptr),
        pimpl(std::make_unique<Impl>())
{
    calculate_viewport(game.framebuffer_width(), game.framebuffer_height());
    // Add components
    add_component(std::make_shared<Camera_Renderer>(game));
    add_component(std::make_shared<Object_Tracker>());
    setup_opengl();
    update_viewport();
}

Camera::~Camera() {}

void Camera::set_size(int width, int height, bool force) {
    if (!force && width == pimpl->screen_size.x && height == pimpl->screen_size.y) return;
    LOGGER_I << "Setting camera size to " << width << "x" << height;
    pimpl->full_screen_texture.reset();
    calculate_viewport(width, height);
    update_viewport();
    pimpl->screen_size.x = width;
    pimpl->screen_size.y = height;
}

void Camera::calculate_viewport(int width, int height) {
    // Calculate viewport rectangle
    auto scale_mode = Configurations::get<std::string>("graphics.scale-mode");
    auto screen_width = static_cast<float>(width);
    auto screen_height = static_cast<float>(height);
    auto game_width = static_cast<float>(game.game_width(false));
    auto game_height = static_cast<float>(game.game_height(false));

    if (scale_mode == "default") {
        // The environment can override "default" at startup if it needs to
        scale_mode = "aspect";
    }

    if (scale_mode == "stretch") {
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.w = screen_width;
        viewport.h = screen_height;
    } else {
        if (scale_mode == "none") {
            viewport.w = std::min(screen_width, game_width);
            viewport.h = std::min(screen_height, game_height);
        } else if (scale_mode == "aspect" ||
                screen_width < game_width || screen_height < game_height) {
            float target_ratio = game_width / game_height;

            viewport.w = screen_width;
            viewport.h = viewport.w / target_ratio;

            if (viewport.h > screen_height) {
                viewport.h = screen_height;
                viewport.w = viewport.h * target_ratio;
            }
        } else {
            int scale_x = static_cast<int>(screen_width / game_width);
            int scale_y = static_cast<int>(screen_height / game_height);
            if (scale_x > scale_y) {
                viewport.w = game_width * scale_y;
                viewport.h = game_height * scale_y;
            } else {
                viewport.w = game_width * scale_x;
                viewport.h = game_height * scale_x;
            }
        }
        viewport.x = screen_width / 2 - viewport.w / 2;
        viewport.y = screen_height / 2 - viewport.h / 2;
    }
}

void Camera::update_viewport(xd::vec2 shake_offset) const {
    pimpl->update_viewport(viewport, shake_offset);
}

void Camera::setup_opengl() const {
    // Setup OpenGL state
    set_clear_color();
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
}

void Camera::set_clear_color() const {
    auto& color = pimpl->clear_color;
    glClearColor(color.r, color.g, color.b, color.a);
}

void Camera::center_at(xd::vec2 pos) {
    position = get_centered_position(pos);
}

void Camera::center_at(const Map_Object& target) {
    position = get_centered_position(target);
}

xd::vec2 Camera::get_centered_position(xd::vec2 pos) const {
    float game_width = static_cast<float>(game.game_width());
    float game_height = static_cast<float>(game.game_height());
    return get_bounded_position(pos - xd::vec2{game_width / 2.0f, game_height / 2.0f});
}

xd::vec2 Camera::get_centered_position(const Map_Object& target) const {
    auto sprite = target.get_sprite();
    auto sprite_size = sprite ? sprite->get_size() : xd::vec2{0.0f, 0.0f};
    auto object_pos = target.get_position() + sprite_size / 2.0f;
    return get_centered_position(object_pos);
}

xd::rect Camera::get_position_bounds() const {
    auto map = game.get_map();
    float map_width = static_cast<float>(map->get_pixel_width());
    float map_height = static_cast<float>(map->get_pixel_height());
    float game_width = static_cast<float>(game.game_width());
    float game_height = static_cast<float>(game.game_height());
    return xd::rect{0.0f, 0.0f,
        map_width - game_width, map_height - game_height };
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


    float game_width = static_cast<float>(game.game_width());
    float game_height = static_cast<float>(game.game_height());
    int y = static_cast<int>(game_height - (rect.y + rect.h));
    xd::vec2 scale{custom_viewport.w / game_width,
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
    pimpl->render_shader(game, viewport, geometry, brightness, contrast);
}

void Camera::start_shaking(xd::vec2 strength, xd::vec2 speed) {
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

    auto width = static_cast<float>(game.game_width());
    auto height = static_cast<float>(game.game_height());

    auto cam_pos = get_pixel_position();
    xd::rect rect{cam_pos.x, cam_pos.y, width, height};
    draw_rect(rect, map_tint);
}

void Camera_Renderer::render(Camera& camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto width = static_cast<float>(game.game_width());
    auto height = static_cast<float>(game.game_height());

    auto& geometry = camera.get_geometry();
    // left, right, bottom, top, near, far
    geometry.projection().load(xd::ortho<float>(0, width, height, 0, -1, 1));

    geometry.model_view().identity();
    auto cam_pos = camera.get_pixel_position();
    geometry.model_view().translate(-cam_pos.x, -cam_pos.y, 0);

    if (camera.is_shaking()) {
        camera.update_viewport(camera.shake_offset());
    }

    game.get_map()->render();

    auto tint_color = camera.get_screen_tint();
    if (!check_close(tint_color.a, 0.0f)) {
        xd::rect rect{cam_pos.x, cam_pos.y, width, height};
        camera.draw_rect(rect, tint_color);
    }

    camera.render_shader();
}

void Object_Tracker::update(Camera& camera) {
    auto obj = camera.get_object();
    if (!obj)
        return;
    camera.center_at(*obj);
}

Screen_Shaker::Screen_Shaker(xd::vec2 strength, xd::vec2 speed)
        : strength(strength)
        , speed(speed * (60.0f / Configurations::get<int>("graphics.logic-fps", "debug.logic-fps")))
        , last_direction(1.0f, 1.0f)
        , offset(0.0f, 0.0f) {
    // Randomize the starting direction
    auto max = static_cast<double>(RAND_MAX);
    if (std::rand() / max < 0.5) last_direction.x = -1.0f;
    if (std::rand() / max < 0.5) last_direction.y = -1.0f;
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
