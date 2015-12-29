#include "../include/layer.hpp"
#include "../include/layer_renderer.hpp"
#include "../include/layer_updater.hpp"
#include "../include/utility.hpp"
#include "../include/base64.hpp"
#include "../include/exceptions.hpp"
#include "../include/log.hpp"
#include <zlib.h>
#include <boost/lexical_cast.hpp>

Layer::Layer() : width(0), height(0), opacity(1.0f), visible(true) {}

void Layer::resize(xd::ivec2 new_size) {
    this->width = new_size.x;
    this->height = new_size.y;
}

rapidxml::xml_node<>* Layer::save(rapidxml::xml_document<>& doc,
            const std::string& node_name) {
    auto node = xml_node(doc, node_name);
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
    save_properties(properties, doc, *node);
    return node;
}

void Layer::load(rapidxml::xml_node<>& node) {
    using boost::lexical_cast;
    name = node.first_attribute("name")->value();
    if (node.first_attribute("width"))
        width = lexical_cast<int>(node.first_attribute("width")->value());
    if (node.first_attribute("height"))
        height = lexical_cast<int>(node.first_attribute("height")->value());
    if (auto opacity_node = node.first_attribute("opacity"))
        opacity = lexical_cast<float>(opacity_node->value());
    if (auto visible_node = node.first_attribute("visible"))
        visible = lexical_cast<bool>(visible_node->value());
    read_properties(properties, node);
    auto vert_iter = properties.find("vertex-shader");
    auto frag_iter = properties.find("fragment-shader");
    if (vert_iter != properties.end() && frag_iter != properties.end()) {
        vertex_shader = vert_iter->second;
        fragment_shader = frag_iter->second;
    } else if (vert_iter != properties.end() || frag_iter != properties.end()) {
        LOGGER_W << "Must define both shader types for layer " << name;
    }
}

Layer::~Layer() {}
