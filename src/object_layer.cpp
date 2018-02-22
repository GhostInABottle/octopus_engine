#include "../include/object_layer.hpp"
#include "../include/object_layer_renderer.hpp"
#include "../include/object_layer_updater.hpp"
#include "../include/map_object.hpp"
#include "../include/map.hpp"
#include "../include/utility.hpp"
#include "../include/exceptions.hpp"
#include <boost/lexical_cast.hpp>

rapidxml::xml_node<>* Object_Layer::save(rapidxml::xml_document<>& doc) {
    auto node = Layer::save(doc, "objectgroup");
    std::string color_hex = color_to_hex(color);
    if (color_hex != std::string("ffffffff"))
        node->append_attribute(xml_attribute(doc, "color", color_hex));
    for (auto& object : objects) {
        node->append_node(object->save(doc));
    }
    return node;
}

std::unique_ptr<Layer> Object_Layer::load(rapidxml::xml_node<>& node, Game& game, const Camera& camera, Map& map) {
    using boost::lexical_cast;
    Object_Layer* layer_ptr = new Object_Layer();
    layer_ptr->Layer::load(node);
    if (auto color_node = node.first_attribute("color"))
        layer_ptr->color = hex_to_color(color_node->value());
    else
        layer_ptr->color = xd::vec4(1.0f);

    // Objects
    for (auto object_node = node.first_node("object");
            object_node; object_node = object_node->next_sibling("object")) {
        auto object_ptr = Map_Object::load(*object_node, game);
        auto object = std::shared_ptr<Map_Object>(object_ptr.release());
        map.add_object(object, -1, layer_ptr);
        if (object->get_name().empty()) {
            object->set_name("UNTITLED" + std::to_string(object->get_id()));
        }
    }

    layer_ptr->renderer.reset(new Object_Layer_Renderer(*layer_ptr, camera));
    layer_ptr->updater.reset(new Object_Layer_Updater(layer_ptr));

    return std::unique_ptr<Layer>(layer_ptr);
}
