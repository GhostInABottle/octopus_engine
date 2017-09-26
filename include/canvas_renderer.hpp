#ifndef HPP_CANVAS_RENDERER
#define HPP_CANVAS_RENDERER

#include <xd/config.hpp>
#include <xd/entity.hpp>
#include <xd/graphics/framebuffer.hpp>
#include <xd/graphics/sprite_batch.hpp>
#include "map.hpp"

class Canvas_Renderer : public xd::render_component<Map> {
public:
	Canvas_Renderer(int game_width, int game_height);
    void render(Map& map);
private:
	std::string last_drawn_text;
	int last_drawn_text_time;
	int ms_between_text_refresh;
    xd::sprite_batch batch;
	xd::framebuffer text_framebuffer;
	xd::texture::ptr text_texture;
};

#endif
