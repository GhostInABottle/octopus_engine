#ifndef HPP_OBJECT_LAYER_RENDERER
#define HPP_OBJECT_LAYER_RENDERER

#include "../xd/graphics/types.hpp"
#include "layer_renderer.hpp"

class Object_Layer_Renderer : public Layer_Renderer {
public:
    Object_Layer_Renderer(const Layer& layer, const Camera& camera);
    void render(Map& map) override;
private:
    xd::vec4 default_outline_color;
};

#endif
