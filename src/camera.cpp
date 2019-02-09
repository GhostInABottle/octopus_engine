#include "../include/camera.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/utility.hpp"
#include "../include/configurations.hpp"
#include "../include/log.hpp"
#include "../include/custom_shaders.hpp"
#include <xd/graphics/vertex_batch.hpp>
#include <xd/graphics/shader_program.hpp>
#include <xd/graphics/shaders.hpp>
#include <xd/config.hpp>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <algorithm>

struct Camera::Impl {
    explicit Impl() :
            current_shader(nullptr) {}
    // Update OpenGL viewport
    void update_viewport(xd::rect viewport, float shake_offset = 0.0f) const {
        glViewport(static_cast<int>(viewport.x + shake_offset),
            static_cast<int>(viewport.y),
            static_cast<int>(viewport.w),
            static_cast<int>(viewport.h));
    }
    // Full-screen shader data
    xd::shader_program* current_shader;
    xd::sprite_batch full_screen_batch;
    xd::texture::ptr full_screen_texture;
    // Apply a certain shader
    void set_shader(const std::string& vertex, const std::string& fragment);
    // Render a full-screen shader
    void render_shader(Game& game, const xd::rect& viewport, xd::transform_geometry& geometry);
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
    std::string vsrc, fsrc;
    if (!vertex.empty() && !fragment.empty()) {
        try {
            vsrc = read_file(vertex);
            fsrc = read_file(fragment);
        } catch (const std::runtime_error& err) {
            LOGGER_W << "Error loading shaders: " << err.what();
            vsrc = fsrc = "";
        }
    }
    if (!vsrc.empty()) {
        current_shader = new Custom_Shader(vsrc, fsrc);
        full_screen_batch.set_shader(current_shader);
    } else {
        current_shader = nullptr;
    }
}

void Camera::Impl::render_shader(Game& game, const xd::rect& viewport, xd::transform_geometry& geometry) {
    int w = game.width();
    int h = game.height();
    if (!full_screen_texture)
        full_screen_texture = xd::create<xd::texture>(w, h, nullptr,
            xd::vec4(0), GL_CLAMP, GL_CLAMP);
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
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    full_screen_batch.draw(geometry.mvp(), game.ticks());
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
        tint_color(hex_to_color(Configurations::get<std::string>("startup.tint-color"))),
        object(nullptr),
        shaker(nullptr),
        pimpl(new Impl())
{
    calculate_viewport(game.width(), game.height());
    // Add components
    add_component(xd::create<Camera_Renderer>(game));
    add_component(xd::create<Object_Tracker>());
    setup_opengl();
    update_viewport();
}

void Camera::calculate_viewport(int width, int height) {
    // Calculate viewport rectangle
    auto scale_mode = Configurations::get<std::string>("game.scale-mode");
    auto screen_width = static_cast<float>(width);
    auto screen_height = static_cast<float>(height);
    auto game_width = static_cast<float>(game.game_width());
    auto game_height = static_cast<float>(game.game_height());
    if (scale_mode == "stretch") {
        viewport.w = screen_width;
        viewport.h = screen_height;
    } else {
        if (scale_mode == "none") {
            viewport.w = std::min(screen_width, game_width * game.get_magnification());
            viewport.h = std::min(screen_height, game_height * game.get_magnification());
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

void Camera::update_viewport(float shake_offset) const {
    pimpl->update_viewport(viewport, shake_offset);
}

void Camera::setup_opengl() const {
    // Setup OpenGL state
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Camera::set_clear_color(xd::vec4 color) const {
    glClearColor(color.r, color.g, color.b, color.a);
}

void Camera::center_at(xd::vec2 pos) {
    set_position(pos - xd::vec2(game.game_width() / 2, game.game_height() / 2));
}

void Camera::center_at(const Map_Object& object) {
    xd::vec2 position = object.get_position();
    auto sprite = object.get_sprite();
    float width = sprite ? sprite->get_size().x : 0.0f;
    float height = sprite ? sprite->get_size().y : 0.0f;
    position.x += width / 2;
    position.y += height / 2;

    center_at(position);
}

void Camera::set_position(xd::vec2 pos) {
    auto map = game.get_map();
    float map_width = static_cast<float>(map->get_pixel_height());
    float map_height = static_cast<float>(map->get_pixel_height());
    float right_limit = map_width - game.game_width();
    float bottom_limit = map_height - game.game_height();
    position = pos;
    if (position.x < 0)
        position.x = 0;
    if (position.x > right_limit)
        position.x = right_limit;
    if (position.y < 0)
        position.y = 0;
    if (position.y > bottom_limit)
        position.y = bottom_limit;
}

void Camera::draw_rect(xd::rect rect, xd::vec4 color, bool fill) {
    GLenum draw_mode = fill ? GL_QUADS :  GL_LINE_LOOP;
    pimpl->draw_quad(geometry.mvp(), rect, color, draw_mode);
}

void Camera::enable_scissor_test(xd::rect rect, xd::rect custom_viewport) {
    if (custom_viewport.w == 0) {
        custom_viewport = viewport;
    }
    xd::vec2 scale{
        custom_viewport.w / game.game_width(),
        custom_viewport.h / game.game_height()
    };
    int y = game.game_height() - static_cast<int>(rect.y + rect.h);
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
    if (!pimpl->current_shader) return;
    pimpl->render_shader(game, viewport, geometry);
}

void Camera::start_shaking(float strength, float speed) {
    auto shaker_comp = xd::create<Screen_Shaker>(strength, speed);
    shaker = static_cast<Screen_Shaker*>(shaker_comp.get());
    add_component(shaker_comp);
}

void Camera::cease_shaking() {
    shaker = nullptr;
    del_component(shaker);
    update_viewport();
}

float Camera::shake_offset() const {
    return is_shaking() ? shaker->shake_offset() : 0.0f;
}

void Camera_Renderer::render(Camera& camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

     auto game_width = static_cast<float>(game.game_width());
     auto game_height = static_cast<float>(game.game_height());

    auto& geometry = camera.get_geometry();
    geometry.projection().load(
        xd::ortho<float>(
            0, game_width,      // left, right
            game_height, 0,     // bottom, top
            -1, 1               // near, far
        )
    );
    geometry.model_view().identity();
    auto cam_pos = camera.get_position();
    geometry.model_view().translate(-cam_pos.x, -cam_pos.y, 0);

    if (camera.is_shaking()) {
        camera.update_viewport(camera.shake_offset());
    }

    game.get_map()->render();

    auto tint_color = camera.get_tint_color();
    if (tint_color.a > 0.0f) {
        auto pos = camera.get_position();
        xd::rect rect{pos.x, pos.y, game_width, game_height};
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

void Screen_Shaker::update(Camera& camera) {
    offset += (strength * speed * direction) / 10.0f;
    if (offset > strength * 2)
        direction = -1;
    else if (offset < -strength * 2)
        direction = 1;
}
