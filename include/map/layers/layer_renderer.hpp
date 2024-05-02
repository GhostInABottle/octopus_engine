#ifndef HPP_LAYER_RENDERER
#define HPP_LAYER_RENDERER

#include "../../map/map.hpp"
#include "../../xd/entity.hpp"
#include "../../xd/graphics/sprite_batch.hpp"

struct Layer;
class Camera;

class Layer_Renderer : public xd::render_component<Map> {
public:
    Layer_Renderer(const Layer& layer, const Camera& camera);
    virtual void render(Map& map) = 0;
    xd::sprite_batch& get_batch() noexcept { return batch; }
    void redraw() noexcept { needs_redraw = true; }
    virtual ~Layer_Renderer() {}
protected:
    xd::sprite_batch batch;
    const Layer& layer;
    const Camera& camera;
    bool needs_redraw;
};

#endif
