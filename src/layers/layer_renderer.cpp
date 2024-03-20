#include "../../include/custom_shaders.hpp"
#include "../../include/layers/layer.hpp"
#include "../../include/layers/layer_renderer.hpp"
#include "../../include/utility/file.hpp"
#include <memory>

Layer_Renderer::Layer_Renderer(const Layer& layer, const Camera& camera) :
        layer(layer), camera(camera), needs_redraw(false) {
    if (layer.vertex_shader.empty()) return;

    auto fs = file_utilities::game_data_filesystem();
    auto vshader = fs->read_file(layer.vertex_shader);
    auto fshader = fs->read_file(layer.fragment_shader);
    batch.set_shader(std::make_unique<Custom_Shader>(vshader, fshader));
}
