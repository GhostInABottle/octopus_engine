#include "../include/layer_renderer.hpp"
#include "../include/layer.hpp"
#include "../include/custom_shaders.hpp"
#include "../include/utility/file.hpp"
#include "../include/log.hpp"
#include <utility>

Layer_Renderer::Layer_Renderer(const Layer& layer, const Camera& camera) :
        layer(layer), camera(camera), needs_redraw(false) {
    if (!layer.vertex_shader.empty()) {
        auto vshader = file_utilities::read_file(layer.vertex_shader);
        auto fshader = file_utilities::read_file(layer.fragment_shader);
        batch.set_shader(std::make_unique<Custom_Shader>(vshader, fshader));
    }
}
