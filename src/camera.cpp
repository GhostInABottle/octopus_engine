#include "../include/camera.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/utility.hpp"
#include "../include/configurations.hpp"
#include <xd/graphics/vertex_batch.hpp>
#include <xd/graphics/shader_program.hpp>
#include <xd/graphics/shaders.hpp>
#include <xd/config.hpp>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

namespace detail {
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
            const xd::vec4& color, GLenum draw_mode = GL_QUADS) {
        static xd::flat_shader shader;
        // Setup uniforms
        mvp = xd::translate(mvp, xd::vec3(rect.x, rect.y, 0.0f));
        shader.setup(mvp, color);
        // Set quad vertex positions
        quad_vertex quad[4];
        quad[0].pos = xd::vec2(0.0f, rect.h);
        quad[1].pos = xd::vec2(rect.w, rect.h);
        quad[2].pos = xd::vec2(rect.w, 0.0f);
        quad[3].pos = xd::vec2(0.0f, 0.0f);
        // Load and render batch
        xd::vertex_batch<quad_vertex_traits> batch(draw_mode);
        batch.load(&quad[0], 4);
        batch.render();
    }

}

Camera::Camera(Game& game) 
        : game(game),
        position(xd::vec2(0.0f, 0.0f)),
        tint_color(hex_to_color(Configurations::get<std::string>("startup.tint-color"))),
        shaker(nullptr)
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
    float screen_width = static_cast<float>(width);
    float screen_height = static_cast<float>(height);
    float game_width = static_cast<float>(game.game_width);
    float game_height = static_cast<float>(game.game_height);
    if (scale_mode == "stretch") {
        viewport.w = screen_width;
        viewport.h = screen_height;
    } else {
        if (scale_mode == "none") {
            viewport.w = game_width * magnification;
            viewport.h = game_height * magnification;
        } else if (scale_mode == "aspect" ||
                screen_width < game_width || screen_height < game_height) {
            float target_ratio = (float)game_width / game_height;

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
    glViewport(static_cast<int>(viewport.x + shake_offset),
        static_cast<int>(viewport.y),
        static_cast<int>(viewport.w),
        static_cast<int>(viewport.h));
}

void Camera::setup_opengl() const {
    // Setup OpenGL state
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Camera::center_at(float x, float y) {
    set_position(x - game.game_width / 2, y - game.game_height / 2);
}

void Camera::set_position(float x, float y) {
    auto map = game.get_map();
    float map_width = static_cast<float>(map->get_width() * map->get_tile_width());
    float map_height = static_cast<float>(map->get_height() * map->get_tile_height());
    float right_limit = map_width - game.game_width;
    float bottom_limit = map_height - game.game_height;
    position.x = x;
    position.y = y;
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
    detail::draw_quad(game.get_mvp(), rect, color, draw_mode);
}

void Camera::enable_scissor_test(xd::rect rect, xd::rect custom_viewport) {
    if (custom_viewport.w == 0)
        custom_viewport = viewport;
    xd::vec2 scale{ custom_viewport.w / game.game_width,
        custom_viewport.h / game.game_height};
    int y = game.game_height - static_cast<int>(rect.y + rect.h);
    glEnable(GL_SCISSOR_TEST);
    glScissor(static_cast<int>(custom_viewport.x + rect.x * scale.x),
        static_cast<int>(custom_viewport.y + y * scale.y),
        static_cast<int>(rect.w * scale.x),
        static_cast<int>(rect.h * scale.y));
}

void Camera::disable_scissor_test() {
    glDisable(GL_SCISSOR_TEST);
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
    if (camera.is_shaking())
        camera.update_viewport(camera.shake_offset());
    game.get_map()->render();
    auto tint_color = camera.get_tint_color();
    if (tint_color.a > 0.0f) {
        auto pos = camera.get_position();
        xd::rect rect(pos.x, pos.y,
            static_cast<float>(game.game_width),
            static_cast<float>(game.game_height));
        camera.draw_rect(rect, tint_color);
    }
}

void Object_Tracker::update(Camera& camera) {
    auto obj = camera.get_object();
    if (!obj)
        return;
    xd::vec2 position = obj->get_position();
    camera.center_at(position.x, position.y);
}

void Screen_Shaker::update(Camera& camera) {
    offset += (strength * speed * direction) / 10.0f;
    if (offset > strength * 2)
        direction = -1;
    else if (offset < -strength * 2)
        direction = 1;
}
