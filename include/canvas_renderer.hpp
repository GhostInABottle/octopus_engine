#ifndef HPP_CANVAS_RENDERER
#define HPP_CANVAS_RENDERER

#include <xd/config.hpp>
#include <xd/entity.hpp>
#include <xd/graphics/sprite_batch.hpp>
#include "map.hpp"

class Canvas_Renderer : public xd::render_component<Map> {
public:
    void render(Map& map);
private:
    xd::sprite_batch batch;
};

#endif
