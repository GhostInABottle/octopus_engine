#include "../include/layer.hpp"
#include "../include/layer_renderer.hpp"
#include "../include/layer_updater.hpp"
#include "../include/utility/math.hpp"
#include "../include/utility/string.hpp"
#include "../include/utility/xml.hpp"
#include "../include/exceptions.hpp"

Layer::Layer() : id(-1), width(0), height(0), opacity(1.0f), visible(true) {}

void Layer::resize(xd::ivec2 new_size) {
    this->width = new_size.x;
    this->height = new_size.y;
}

rapidxml::xml_node<>* Layer::save(rapidxml::xml_document<>& doc,
            const std::string& node_name) {
    auto node = xml_node(doc, node_name);
    if (id != -1)
        node->append_attribute(xml_attribute(doc, "id", std::to_string(id)));
    if (!name.empty())
        node->append_attribute(xml_attribute(doc, "name", name));
    if (width != 0)
        node->append_attribute(xml_attribute(doc, "width", std::to_string(width)));
    if (height != 0)
        node->append_attribute(xml_attribute(doc, "height", std::to_string(height)));
    if (!check_close(opacity, 1.0f))
        node->append_attribute(xml_attribute(doc, "opacity", std::to_string(opacity)));
    if (!visible)
        node->append_attribute(xml_attribute(doc, "visible", "0"));

    properties.save(doc, *node);
    return node;
}

void Layer::load(rapidxml::xml_node<>& node) {
    if (auto name_node = node.first_attribute("name"))
        name = name_node->value();
    else
        throw tmx_exception("Missing layer name");

    if (auto id_node = node.first_attribute("id"))
        id = std::stoi(id_node->value());
    else
        throw tmx_exception("Missing layer ID for layer: " + name);

    if (auto width_node = node.first_attribute("width"))
        width = std::stoi(width_node->value());

    if (auto height_node = node.first_attribute("height"))
        height = std::stoi(height_node->value());

    if (auto opacity_node = node.first_attribute("opacity"))
        opacity = std::stof(opacity_node->value());

    if (auto visible_node = node.first_attribute("visible"))
        visible = string_utilities::string_to_bool(visible_node->value());

    properties.read(node);

    auto has_vert = properties.contains("vertex-shader");
    auto has_frag = properties.contains("fragment-shader");
    if (has_vert && has_frag) {
        vertex_shader = properties["vertex-shader"];
        fragment_shader = properties["fragment-shader"];
    } else if (has_vert || has_frag) {
        throw tmx_exception("Must define both shader types for layer " + name);
    }
}

Layer::~Layer() {}
