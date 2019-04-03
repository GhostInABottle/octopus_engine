#ifndef HPP_OBJECT_LAYER_RENDERER
#define HPP_OBJECT_LAYER_RENDERER

#include "layer_renderer.hpp"

class Object_Layer_Renderer : public Layer_Renderer {
public:
    Object_Layer_Renderer(const Layer& layer, const Camera& camera);
    void render(Map& map) override;
};

#endif
