#include "../include/layer_renderer.hpp"
#include "../include/layer.hpp"
#include "../include/custom_shaders.hpp"
#include "../include/utility.hpp"
#include "../include/log.hpp"

Layer_Renderer::Layer_Renderer(const Layer& layer, const Camera& camera) :
        layer(layer), camera(camera), needs_redraw(false) {
    if (!layer.vertex_shader.empty()) {
        auto vshader = read_file(layer.vertex_shader);
        auto fshader = read_file(layer.fragment_shader);
        batch.set_shader(new Custom_Shader(vshader, fshader));
    }
}
