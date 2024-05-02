#include "../../include/map/tmx_properties.hpp"
#include "../../include/utility/xml.hpp"
#include "../../include/utility/string.hpp"

void Tmx_Properties::read(rapidxml::xml_node<>& parent_node) {
    auto props_node = parent_node.first_node("properties");
    if (!props_node) return;
    for (auto prop_node = props_node->first_node("property");
            prop_node; prop_node = prop_node->next_sibling("property")) {
        std::string name = prop_node->first_attribute("name")->value();
        std::string value = prop_node->first_attribute("value")->value();
        properties[name] = value;
        ordered_keys.push_back(name);
    }
}

void Tmx_Properties::save(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node) {
    if (properties.empty())
        return;
    auto properties_node = xml_node(doc, "properties");
    node.append_node(properties_node);
    for (auto& key : ordered_keys) {
        auto property_node = xml_node(doc, "property");
        auto name_attr = xml_attribute(doc, "name", key);
        auto value_attr = xml_attribute(doc, "value", properties[key]);
        property_node->append_attribute(name_attr);
        property_node->append_attribute(value_attr);
        properties_node->append_node(property_node);
    }
}

void Tmx_Object::set_editor_property(const std::string& prop_name, const std::string& value, const std::string& default_value, bool capitalize_value) {
    auto old_value = string_utilities::capitalize(get_property(prop_name));
    auto cap_value = string_utilities::capitalize(value);
    if (cap_value == old_value) return;
    if (cap_value == string_utilities::capitalize(default_value)) {
        erase_property(prop_name);
        return;
    }
    set_property(prop_name, capitalize_value ? cap_value : value);
}
