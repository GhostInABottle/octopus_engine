#include "../../custom_shaders.hpp"
#include "../../exceptions.hpp"
#include "../../utility/file.hpp"
#include "../../utility/string.hpp"
#include "layer.hpp"
#include "layer_renderer.hpp"
#include <exception>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

namespace {
    static bool uniform_string_to_number(const std::string& value, float& result) {
        try {
            result = std::stof(value.c_str());
            return true;
        } catch (std::exception&) {
            return false;
        }
    }
}

Layer_Renderer::Layer_Renderer(const Layer& layer, const Camera& camera) :
        layer(layer), camera(camera), needs_redraw(false) {
    if (!layer.has_vertex_shader()) return;

    auto fs = file_utilities::game_data_filesystem();
    auto v_shader = fs->read_file(layer.get_vertex_shader());
    auto f_shader = fs->read_file(layer.get_fragment_shader());
    auto shader = std::make_unique<Custom_Shader>(v_shader, f_shader);

    std::unordered_set<std::string> vector_uniforms;
    const auto& properties = layer.properties;
    // Load any uniforms
    for (const auto& [name, value] : properties) {
        if (!string_utilities::starts_with(name, "uniform-")) continue;

        float result = 0.0f;
        auto parts = string_utilities::split(name, "-");
        if (parts.size() == 3) {
            vector_uniforms.insert(parts[1]);
        } else if (parts.size() == 2 && uniform_string_to_number(value, result)) {
            if (value.find('.') == std::string::npos) {
                batch.set_uniform(parts[1], static_cast<int>(result));
            } else {
                batch.set_uniform(parts[1], result);
            }
        } else {
            throw tmx_exception{"Unexpected " + name + " value: " + value};
        }
    }

    for (const auto& name : vector_uniforms) {
        float x = 0.0f;
        auto x_name = "uniform-" + name + "-x";
        if (!properties.contains(x_name) || !uniform_string_to_number(properties[x_name], x)) {
            throw tmx_exception{ "Missing x coordinate for uniform " + name};
        }
        float y = 0.0f;
        auto y_name = "uniform-" + name + "-y";
        if (!properties.contains(y_name) || !uniform_string_to_number(properties[y_name], y)) {
            throw tmx_exception{ "Missing y coordinate for uniform " + name };
        }

        auto z_name = "uniform-" + name + "-z";
        auto w_name = "uniform-" + name + "-w";
        if (!properties.contains(z_name) && !properties.contains(w_name)) {
            batch.set_uniform(name, xd::vec2(x, y));
            continue;
        }

        float z = 0.0f;
        if (!properties.contains(z_name) || !uniform_string_to_number(properties[z_name], z)) {
            throw tmx_exception{ "Missing z coordinate for uniform " + name };
        }
        if (!properties.contains(w_name)) {
            batch.set_uniform(name, xd::vec3(x, y, z));
            continue;
        }

        float w = 0.0f;
        if (!uniform_string_to_number(properties[w_name], w)) {
            throw tmx_exception{ "Missing w coordinate for uniform " + name };
        }
        batch.set_uniform(name, xd::vec4(x, y, z, w));
    }

    batch.set_shader(std::move(shader));
}
