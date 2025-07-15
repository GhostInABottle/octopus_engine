#ifndef HPP_IMAGE_LAYER_RENDERER
#define HPP_IMAGE_LAYER_RENDERER

#include "layer_renderer.hpp"

class Image_Layer_Renderer : public Layer_Renderer {
public:
    Image_Layer_Renderer(const Layer& layer, const Camera& camera)
        : Layer_Renderer(layer, camera) {}
    void render(Map& map) override;
};

#endif
