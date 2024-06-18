#include "../../../include/custom_shaders.hpp"
#include "../../../include/map/layers/layer.hpp"
#include "../../../include/map/layers/layer_renderer.hpp"
#include "../../../include/utility/file.hpp"
#include <memory>

Layer_Renderer::Layer_Renderer(const Layer& layer, const Camera& camera) :
        layer(layer), camera(camera), needs_redraw(false) {
    if (!layer.has_vertex_shader()) return;

    auto fs = file_utilities::game_data_filesystem();
    auto v_shader = fs->read_file(layer.get_vertex_shader());
    auto f_shader = fs->read_file(layer.get_fragment_shader());
    batch.set_shader(std::make_unique<Custom_Shader>(v_shader, f_shader));
}
