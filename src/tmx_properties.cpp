#include "../include/tmx_properties.hpp"
#include "../include/utility.hpp"

void Tmx_Properties::read(rapidxml::xml_node<>& parent_node) {
    if (auto props_node = parent_node.first_node("properties")) {
        for (auto prop_node = props_node->first_node("property");
            prop_node; prop_node = prop_node->next_sibling("property")) {
            std::string name = prop_node->first_attribute("name")->value();
            std::string value = prop_node->first_attribute("value")->value();
            properties[name] = value;
        }
    }
}

void Tmx_Properties::save(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node) {
    if (properties.empty())
        return;
    auto properties_node = xml_node(doc, "properties");
    node.append_node(properties_node);
    for (auto& prop : properties) {
        auto property_node = xml_node(doc, "property");
        auto name_attr = xml_attribute(doc, "name", prop.first);
        auto value_attr = xml_attribute(doc, "value", prop.second);
        property_node->append_attribute(name_attr);
        property_node->append_attribute(value_attr);
        properties_node->append_node(property_node);
    }
}