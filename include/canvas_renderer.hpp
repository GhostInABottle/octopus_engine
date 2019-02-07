#ifndef HPP_CANVAS_RENDERER
#define HPP_CANVAS_RENDERER

#include <xd/config.hpp>
#include <xd/entity.hpp>
#include <xd/graphics/framebuffer.hpp>
#include <xd/graphics/sprite_batch.hpp>
#include <xd/graphics/types.hpp>
#include "map.hpp"

class Game;
class Camera;
class Canvas;

class Canvas_Renderer : public xd::render_component<Map> {
public:
    Canvas_Renderer(Game& game, Camera& camera);
    void render(Map& map);
private:
    void setup_framebuffer(Canvas* canvas);
    void render_framebuffer(Canvas* canvas);
    void render_canvas(Canvas* canvas, Canvas* parent = nullptr);
    void render_text(Canvas* canvas, Canvas* parent = nullptr);
    void render_image(Canvas* canvas, Canvas* parent = nullptr);
    bool should_redraw(Canvas* canvas);
    Game& game;
    Camera& camera;
    std::string last_drawn_text;
    xd::sprite_batch batch;
    bool drawn_to_batch;
    bool fbo_supported;
};

#endif
