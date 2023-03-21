#include "../../include/utility/xml.hpp"

rapidxml::xml_node<>* xml_node(rapidxml::xml_document<>& doc,
    const std::string& name, const std::string& value,
    rapidxml::node_type type) {
    char* name_str = doc.allocate_string(name.c_str());
    char* value_str = nullptr;
    if (!value.empty())
        value_str = doc.allocate_string(value.c_str());
    return doc.allocate_node(type, name_str, value_str);
}

rapidxml::xml_attribute<>* xml_attribute(rapidxml::xml_document<>& doc,
    const std::string& name, const std::string& value) {
    char* name_str = doc.allocate_string(name.c_str());
    char* value_str = doc.allocate_string(value.c_str());
    return doc.allocate_attribute(name_str, value_str);
}