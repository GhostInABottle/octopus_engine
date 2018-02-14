#include "../include/canvas_renderer.hpp"
#include "../include/canvas.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/camera.hpp"
#include "../include/sprite_data.hpp"
#include "../include/configurations.hpp"

Canvas_Renderer::Canvas_Renderer()
    : ms_between_text_refresh(1000 / Configurations::get<int>("debug.text-fps")) {}

void Canvas_Renderer::render(Map& map) {
    auto& game = map.get_game();
    auto camera_pos = game.get_camera()->get_position();
    auto& canvases = map.get_canvases();
    for (auto& canvas : canvases) {
        if (canvas->is_visible()) {
            xd::vec2 pos = canvas->get_position();
            float x = pos.x;
            float y = pos.y;
            std::string text = canvas->get_text();
            // Limit drawing to scissor rectangle, if specified
            xd::rect scissor = canvas->get_scissor_box();
            bool has_scissor_box = scissor.w > 0 && scissor.h > 0;
            if (has_scissor_box) {
                game.get_camera()->enable_scissor_test(scissor);
            }
            if (!text.empty()) {
                // Text rendering
                bool framebuffer_supported = xd::framebuffer::extension_supported();
                bool time_to_update = game.ticks() - last_drawn_text_time > ms_between_text_refresh;
                // The first time text is rendered, we render it to a texture
                // if it's not supported, we render directly every frame (slow)
                if (text != last_drawn_text || !framebuffer_supported || time_to_update) {
                    if (framebuffer_supported) {
                        if (has_scissor_box) {
                            // Use a texture-sized viewport when calculating scissor box
                            xd::rect viewport = xd::rect(0, 0, game.game_width, game.game_height);
                            game.get_camera()->enable_scissor_test(scissor, viewport);
                        }
                        text_framebuffer.attach_color_texture(canvas->get_texture(), 0);
                        text_framebuffer.bind();
                        glViewport(0, 0, game.game_width, game.game_height);
                        glPushAttrib(GL_COLOR_BUFFER_BIT);
                        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glPopAttrib();
                        auto fb_complete = text_framebuffer.check_complete();
                        if (!std::get<0>(fb_complete))
                            throw std::exception(std::get<1>(fb_complete).c_str());
                    }

                    auto style = canvas->get_style();
                    style->color().a = canvas->get_opacity();
                    auto& lines = canvas->get_text_lines();
                    for (auto line : lines) {
                        float draw_x = x;
                        float draw_y = y;
                        if (!canvas->is_camera_relative_text()) {
                            draw_x -= camera_pos.x;
                            draw_y -= camera_pos.y;
                        }

                        canvas->render_text(line, draw_x, game.game_height - draw_y);
                        y += style->line_height();
                    }
                    text_framebuffer.unbind();
                    last_drawn_text = text;
                    last_drawn_text_time = game.ticks();
                }
                // Render the previously created texture
                if (framebuffer_supported)
                {
                    if (has_scissor_box) {
                        // Scissor test was already applied when drawing to FBO
                        game.get_camera()->disable_scissor_test();
                        has_scissor_box = false;
                    }
                    glDisable(GL_BLEND);
                    batch.clear();
                    xd::vec4 color(1.0f, 1.0f, 1.0f, canvas->get_opacity());
                    batch.add(canvas->get_texture(), 0, 0, color);
                    xd::transform_geometry geometry;
                    geometry.projection().push(
                        xd::ortho<float>(
                            0, static_cast<float>(game.game_width), // left, right
                            0, static_cast<float>(game.game_height), // bottom, top
                            -1, 1 // near, far
                            )
                    );
                    geometry.model_view().push(xd::mat4());
                    game.get_camera()->update_viewport();
                    batch.draw(geometry.mvp());
                    glEnable(GL_BLEND);
                }
            } else {
                // Texture rendering
                batch.clear();
                x += camera_pos.x;
                y += camera_pos.y;
                if (auto sprite = canvas->get_sprite())
                    sprite->render(batch, xd::vec2(x, y), canvas->get_opacity());
                else {
                    xd::vec4 color(1.0f, 1.0f, 1.0f, canvas->get_opacity());
                    batch.add(canvas->get_texture(), x, y, xd::radians(canvas->get_angle()),
                        canvas->get_magnification(), color, canvas->get_origin());
                }
                batch.draw(game.get_mvp());
            }
            if (has_scissor_box) {
                game.get_camera()->disable_scissor_test();
            }
        }
    }
}
