#ifndef HPP_CANVAS_RENDERER
#define HPP_CANVAS_RENDERER

#include <xd/config.hpp>
#include <xd/entity.hpp>
#include <xd/graphics/framebuffer.hpp>
#include <xd/graphics/sprite_batch.hpp>
#include <xd/graphics/types.hpp>
#include "map.hpp"

class Game;
class Canvas;

class Canvas_Renderer : public xd::render_component<Map> {
public:
    Canvas_Renderer();
    void render(Map& map);
private:
    void setup_framebuffer(Game& game, Canvas* canvas);
    void render_framebuffer(Game& game, Canvas* canvas);
    void render_canvas(Game& game, Canvas* canvas, Canvas* parent = nullptr);
    void render_text(Game& game, Canvas* canvas, Canvas* parent = nullptr);
    void render_image(Game& game, Canvas* canvas, Canvas* parent = nullptr);
    bool should_redraw(Game& game, Canvas* canvas);
    std::string last_drawn_text;
    xd::sprite_batch batch;
    bool drawn_to_batch;
    bool fbo_supported;
};

#endif
