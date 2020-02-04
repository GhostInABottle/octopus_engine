#ifndef HPP_UTILITY_XML
#define HPP_UTILITY_XML

#include <string>
#include "../vendor/rapidxml.hpp"

// Allocate an XML node
rapidxml::xml_node<>* xml_node(rapidxml::xml_document<>& doc,
    const std::string& name, const std::string& value = "",
    rapidxml::node_type type = rapidxml::node_element);
// Allocate an XML attribute
rapidxml::xml_attribute<>* xml_attribute(rapidxml::xml_document<>& doc,
    const std::string& name, const std::string& value);

#endif
