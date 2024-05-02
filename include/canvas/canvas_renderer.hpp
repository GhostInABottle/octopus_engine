#ifndef HPP_CANVAS_RENDERER
#define HPP_CANVAS_RENDERER

#include "../map/map.hpp"
#include "../xd/entity.hpp"
#include "../xd/graphics/sprite_batch.hpp"
#include "../xd/graphics/types.hpp"

class Game;
class Camera;
class Base_Canvas;

class Canvas_Renderer : public xd::render_component<Map> {
public:
    Canvas_Renderer(Game& game, Camera& camera);
    void render(Map& map);
private:
    void setup_framebuffer(const Base_Canvas& canvas);
    void render_framebuffer(const Base_Canvas& canvas, const Base_Canvas& root);
    void render_canvas(Base_Canvas& canvas, Base_Canvas* parent = nullptr, Base_Canvas* root = nullptr);
    void render_background(Base_Canvas& canvas, Base_Canvas* parent = nullptr);
    bool should_redraw(const Base_Canvas& canvas);
    void draw(const xd::mat4 mvp, const Base_Canvas& root);
    Game& game;
    Camera& camera;
    std::string last_drawn_text;
    xd::sprite_batch batch;
    bool fbo_supported;
    xd::rect background_margins;
};

#endif
