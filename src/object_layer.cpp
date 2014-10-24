#include "../include/object_layer.hpp"
#include "../include/object_layer_renderer.hpp"
#include "../include/object_layer_updater.hpp"
#include "../include/map_object.hpp"
#include "../include/map.hpp"
#include "../include/utility.hpp"
#include "../include/exceptions.hpp"
#include <boost/lexical_cast.hpp>

std::unique_ptr<Layer> Object_Layer::load(rapidxml::xml_node<>& node, Game& game, const Camera& camera, Map& map) {
    using boost::lexical_cast;
    Object_Layer* layer_ptr = new Object_Layer();
    layer_ptr->name = node.first_attribute("name")->value();
    if (node.first_attribute("width"))
        layer_ptr->width = lexical_cast<int>(node.first_attribute("width")->value());
    if (node.first_attribute("height"))
        layer_ptr->height = lexical_cast<int>(node.first_attribute("height")->value());
    if (auto opacity_node = node.first_attribute("opacity"))
        layer_ptr->opacity = lexical_cast<float>(opacity_node->value());
    if (auto visible_node = node.first_attribute("visible"))
        layer_ptr->visible = lexical_cast<bool>(visible_node->value());
    if (auto color_node = node.first_attribute("color"))
        layer_ptr->color = hex_to_color(color_node->value());
    else
        layer_ptr->color = hex_to_color("a0a0a4");

    // Layer properties
    read_properties(layer_ptr->properties, node);

    // Objects
    for (auto object_node = node.first_node("object");
            object_node; object_node = object_node->next_sibling("object")) {
        auto object_ptr = Map_Object::load(*object_node, game, map.get_asset_manager());
        if (object_ptr->get_name().empty())
            object_ptr->set_name("unnamed " +
                lexical_cast<std::string>(map.object_count()));
        auto object = std::shared_ptr<Map_Object>(object_ptr.release());
        map.add_object(object, -1, layer_ptr);
    }

    layer_ptr->renderer.reset(new Object_Layer_Renderer(*layer_ptr, camera));
    layer_ptr->updater.reset(new Object_Layer_Updater(layer_ptr));

    return std::unique_ptr<Layer>(layer_ptr);
}
