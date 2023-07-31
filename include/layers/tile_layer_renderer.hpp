#ifndef HPP_TILE_LAYER_RENDERER
#define HPP_TILE_LAYER_RENDERER

#include "../xd/graphics/sprite_batch.hpp"
#include "layer_renderer.hpp"

class Tile_Layer_Renderer : public Layer_Renderer {
public:
    Tile_Layer_Renderer(const Layer& layer, const Camera& camera)
        : Layer_Renderer(layer, camera) {}
    void render(Map& map) override;
private:
    xd::sprite_batch::batch_list cache;
};

#endif
