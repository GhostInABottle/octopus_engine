#ifndef HPP_LAYER_RENDERER
#define HPP_LAYER_RENDERER

#include <xd/config.hpp>
#include <xd/entity.hpp>
#include <xd/graphics/sprite_batch.hpp>
#include <xd/graphics/types.hpp>
#include "map.hpp"

struct Layer;
class Camera;

class Layer_Renderer : public xd::render_component<Map> {
public:
    Layer_Renderer(const Layer& layer, const Camera& camera) : layer(layer),
        camera(camera), needs_redraw(false) {}
    virtual void render(Map& map) = 0;
    xd::sprite_batch& get_batch() { return batch; }
    void redraw() { needs_redraw = true; }
protected:
    xd::sprite_batch batch;
    const Layer& layer;
    const Camera& camera;
    bool needs_redraw;
};

#endif
