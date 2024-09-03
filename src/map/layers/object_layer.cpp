#include "../../../include/map/layers/object_layer.hpp"
#include "../../../include/map/layers/object_layer_renderer.hpp"
#include "../../../include/map/layers/object_layer_updater.hpp"
#include "../../../include/map/map_object.hpp"
#include "../../../include/map/map.hpp"
#include "../../../include/utility/color.hpp"
#include "../../../include/utility/xml.hpp"
#include "../../../include/exceptions.hpp"
#include <algorithm>
#include <stdexcept>

rapidxml::xml_node<>* Object_Layer::save(rapidxml::xml_document<>& doc) {
    auto node = Layer::save(doc, "objectgroup");
    std::string color_hex = color_to_hex(tint_color, true);
    if (color_hex != "ffffffff")
        node->append_attribute(xml_attribute(doc, "tintcolor", color_hex));
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

    if (auto color_node = node.first_attribute("tintcolor")) {
        try {
            layer_ptr->tint_color = string_to_color(color_node->value());
        } catch (std::runtime_error& error) {
            throw tmx_exception(std::string{"Error setting object tint color to "}
                + color_node->value() + ": " + error.what());
        }
    } else {
        layer_ptr->tint_color = xd::vec4(1.0f);
    }

    // Objects
    for (auto object_node = node.first_node("object");
            object_node; object_node = object_node->next_sibling("object")) {
        try {
            map.add_object(Map_Object::load(*object_node, game), layer_ptr.get());
        } catch (std::runtime_error& error) {
            throw tmx_exception(error.what());
        }
    }

    layer_ptr->renderer = std::make_unique<Object_Layer_Renderer>(*layer_ptr, camera);
    layer_ptr->updater = std::make_unique<Object_Layer_Updater>(*layer_ptr);

    return layer_ptr;
}
