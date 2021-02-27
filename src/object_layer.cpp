#include "../include/object_layer.hpp"
#include "../include/object_layer_renderer.hpp"
#include "../include/object_layer_updater.hpp"
#include "../include/map_object.hpp"
#include "../include/map.hpp"
#include "../include/utility/color.hpp"
#include "../include/utility/xml.hpp"
#include "../include/exceptions.hpp"
#include <algorithm>

rapidxml::xml_node<>* Object_Layer::save(rapidxml::xml_document<>& doc) {
    auto node = Layer::save(doc, "objectgroup");
    std::string color_hex = color_to_hex(color, true);
    if (color_hex != "ffffffff")
        node->append_attribute(xml_attribute(doc, "color", color_hex));
    auto sorted_objects{objects};
    std::sort(sorted_objects.begin(), sorted_objects.end(),
        [](Map_Object* a, Map_Object* b) { return a->get_id() < b->get_id(); });
    for (auto& object : sorted_objects) {
        node->append_node(object->save(doc));
    }
    return node;
}

std::unique_ptr<Layer> Object_Layer::load(rapidxml::xml_node<>& node, Game& game, const Camera& camera, Map& map) {
    auto layer_ptr = std::make_unique<Object_Layer>();
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
        map.add_object(object, layer_ptr.get());
        if (object->get_name().empty()) {
            object->set_name("UNTITLED" + std::to_string(object->get_id()));
        }
    }

    layer_ptr->renderer = std::make_unique<Object_Layer_Renderer>(*layer_ptr, camera);
    layer_ptr->updater = std::make_unique<Object_Layer_Updater>(*layer_ptr);

    return layer_ptr;
}
