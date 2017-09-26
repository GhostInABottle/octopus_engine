#include "../include/canvas_renderer.hpp"
#include "../include/canvas.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/camera.hpp"
#include "../include/sprite_data.hpp"
#include "../include/configurations.hpp"

Canvas_Renderer::Canvas_Renderer(int game_width, int game_height)
	: ms_between_text_refresh(1000 / Configurations::get<int>("debug.text-fps"))
{
	if (xd::framebuffer::extension_supported()) {
		text_texture = xd::create<xd::texture>(
				game_width,
				game_height,
				nullptr,
				xd::vec4(0),
				GL_CLAMP,
				GL_CLAMP,
				GL_NEAREST,
				GL_NEAREST);
	}
}

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
            if (!text.empty()) {
				bool framebuffer_supported = xd::framebuffer::extension_supported();
				bool time_to_update = game.ticks() - last_drawn_text_time > ms_between_text_refresh;
				// The first time text is rendered, we render it to a texture
				// if it's not supported, we render directly every frame (slow)
				if (text != last_drawn_text || !framebuffer_supported || time_to_update) {
					if (framebuffer_supported) {
						text_framebuffer.attach_color_texture(text_texture, 0);
						text_framebuffer.bind();
						glViewport(0, 0, game.width(), game.height());
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
						canvas->render_text(line, x, game.game_height - y);
						y += style->line_height();
					}
					text_framebuffer.unbind();
					last_drawn_text = text;
					last_drawn_text_time = game.ticks();
				}
				// Render the previously created texture
				if (framebuffer_supported)
				{
					glDisable(GL_BLEND);
					batch.clear();
					xd::vec4 color(1.0f, 1.0f, 1.0f, canvas->get_opacity());
					batch.add(text_texture, 0, 0, color);
					xd::transform_geometry geometry;
					geometry.projection().push(
						xd::ortho<float>(
							0, static_cast<float>(game.width()), // left, right
							0, static_cast<float>(game.height()), // bottom, top
							-1, 1 // near, far
							)
					);
					geometry.model_view().push(xd::mat4());
					glViewport(0, 0, game.width(), game.height());
					batch.draw(geometry.mvp());
					game.get_camera()->update_viewport();
					glEnable(GL_BLEND);
				}
            } else {
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
        }
    }
}
