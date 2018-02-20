#include "../include/canvas_renderer.hpp"
#include "../include/canvas.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/camera.hpp"
#include "../include/sprite_data.hpp"
#include "../include/configurations.hpp"
#include "../include/utility.hpp"

Canvas_Renderer::Canvas_Renderer() {
    ms_between_refresh = 1000 / Configurations::get<int>("debug.canvas-fps");
    framebuffer_supported = Configurations::get<bool>("debug.use-fbo")
        && xd::framebuffer::extension_supported();
}

void Canvas_Renderer::render(Map& map) {
    auto& game = map.get_game();
    auto& canvases = map.get_canvases();
    for (auto& canvas : canvases) {
        if (canvas->is_visible()) {
            // Limit drawing to scissor rectangle, if specified
            xd::rect scissor = canvas->get_scissor_box();
            bool has_scissor_box = scissor.w > 0;
            if (has_scissor_box) {
                game.get_camera()->enable_scissor_test(scissor);
            }
            batch.clear();
            xd::mat4 mvp = game.get_mvp();
            bool text_canvas = canvas->get_type() == Canvas::Type::TEXT;
            bool time_to_update = game.ticks() - last_drawn_time > ms_between_refresh;
            // The first time text is rendered, we render it to a texture
            // if it's not supported, we render directly every frame (slower)
            if (canvas->should_redraw() || !framebuffer_supported || time_to_update) {
                if (text_canvas && framebuffer_supported) {
                    setup_framebuffer(game, canvas.get());
                }
                render_canvas(game, canvas.get());
                canvas->get_framebuffer().unbind();
                last_drawn_time = game.ticks();
            }
            // Render the previously created texture
            if (text_canvas && framebuffer_supported)
            {
                if (has_scissor_box) {
                    // Scissor test was already applied when drawing to FBO
                    game.get_camera()->disable_scissor_test();
                    has_scissor_box = false;
                }
                render_framebuffer(game, canvas.get());
                xd::transform_geometry geometry;
                geometry.projection().push(
                    xd::ortho<float>(
                        0, static_cast<float>(game.game_width), // left, right
                        0, static_cast<float>(game.game_height), // bottom, top
                        -1, 1 // near, far
                        )
                );
                geometry.model_view().push(xd::mat4());
                mvp = geometry.mvp();
            }

            if (drawn_to_batch) {
                batch.draw(mvp);
            }

            glEnable(GL_BLEND);
            if (has_scissor_box) {
                game.get_camera()->disable_scissor_test();
            }
        }
    }
}

void Canvas_Renderer::setup_framebuffer(Game& game, Canvas* canvas) {
    auto scissor_box = canvas->get_scissor_box();
    if (scissor_box.w > 0) {
        // Use a texture-sized viewport when calculating scissor box
        xd::rect viewport = xd::rect(0, 0, game.game_width, game.game_height);
        game.get_camera()->enable_scissor_test(scissor_box, viewport);
    }
    auto& framebuffer = canvas->get_framebuffer();
    framebuffer.attach_color_texture(canvas->get_fbo_texture(), 0);
    framebuffer.bind();
    glViewport(0, 0, game.game_width, game.game_height);
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPopAttrib();
    auto fb_complete = framebuffer.check_complete();
    if (!std::get<0>(fb_complete))
        throw std::exception(std::get<1>(fb_complete).c_str());
}

void Canvas_Renderer::render_framebuffer(Game& game, Canvas* canvas) {
    glDisable(GL_BLEND);
    batch.add(canvas->get_fbo_texture(), 0, 0, xd::vec4(1.0f));
    drawn_to_batch = true;
    game.get_camera()->update_viewport();
    canvas->mark_redrawn();
}

void Canvas_Renderer::render_canvas(Game& game, Canvas* canvas, Canvas* parent) {
    if (canvas->get_type() == Canvas::Type::TEXT) {
        render_text(game, canvas, parent);
    } else {
        render_image(game, canvas, parent);
    }
    for (size_t i = 0; i < canvas->child_count(); ++i) {
        render_canvas(game, canvas->get_child(i), canvas);
    }
    canvas->mark_redrawn();
}

void Canvas_Renderer::render_text(Game& game, Canvas* canvas, Canvas* parent) {
    auto style = canvas->get_style();
    style->color().a = canvas->get_opacity();
    auto& lines = canvas->get_text_lines();
    xd::vec2 pos = canvas->get_position();
    auto camera_pos = game.get_camera()->get_position();
    for (auto line : lines) {
        float draw_x = pos.x;
        float draw_y = pos.y;
        if (!canvas->is_camera_relative_text()) {
            draw_x -= camera_pos.x;
            draw_y -= camera_pos.y;
        }

        canvas->render_text(line, draw_x, game.game_height - draw_y);
        pos.y += style->line_height();
    }
}

void Canvas_Renderer::render_image(Game& game, Canvas* canvas, Canvas* parent) {
    xd::vec2 pos = canvas->get_position();
    auto camera_pos = game.get_camera()->get_position();
    pos.x += camera_pos.x;
    pos.y += camera_pos.y;
    xd::vec4 color = canvas->get_color();
    if (parent) {
        pos += parent->get_position();
        if (check_close(color, xd::vec4(1.0f)))
            color = parent->get_color();
    }
    if (auto sprite = canvas->get_sprite()) {
        sprite->render(batch, xd::vec2(pos.x, pos.y), 1.0f, color);
    } else {
        float angle = canvas->get_angle();
        xd::vec2 magnification = canvas->get_magnification();
        if (parent) {
            if (check_close(angle, 0.0f))
                angle = parent->get_angle();
            if (check_close(magnification, xd::vec2(1.0f)))
                magnification = parent->get_magnification();
        }
        batch.add(canvas->get_image_texture(), pos.x, pos.y,
            xd::radians(angle), magnification, color, canvas->get_origin());
    }
    drawn_to_batch = true;
}
