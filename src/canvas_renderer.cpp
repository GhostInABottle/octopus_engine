#include "../include/canvas_renderer.hpp"
#include "../include/canvas.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/camera.hpp"
#include "../include/sprite_data.hpp"

void Canvas_Renderer::render(Map& map) {
    auto& game = map.get_game();
    auto camera_pos = game.get_camera()->get_position();
    auto& canvases = map.get_canvases();
    for (auto& canvas : canvases) {
        if (canvas->is_visible()) {
            xd::vec2 pos = canvas->get_position();
            float x = pos.x;
            float y = pos.y;
            if (!canvas->get_text().empty()) {
                auto style = canvas->get_style();
                style->color().a = canvas->get_opacity();
                auto& lines = canvas->get_text_lines();
                for (auto line : lines) {
                    canvas->render_text(line, x, game.game_height - y);
                    y += style->line_height();
                }
            } else {
                batch.clear();
                x += camera_pos.x;
                y += camera_pos.y;
                if (auto sprite = canvas->get_sprite())
                    sprite->render(batch, xd::vec2(x, y), canvas->get_opacity());
                else {
                    xd::vec4 color(1.0f, 1.0f, 1.0f, canvas->get_opacity());
                    batch.add(canvas->get_texture(), x, y, canvas->get_angle(),
                    canvas->get_magnification(), color, canvas->get_origin());
                }
                batch.draw(game.get_mvp());
            }
        }
    }
}
