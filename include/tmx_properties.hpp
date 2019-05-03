#ifndef HPP_TMX_PROPERTIES
#define HPP_TMX_PROPERTIES

#include "rapidxml.hpp"
#include <string>
#include <unordered_map>

class Tmx_Properties {
public:
    bool has_property(const std::string& key) const {
        return properties.find(key) != properties.end();
    }
    std::string& operator[](const std::string& key) {
        return properties[key];
    }
    std::string operator[](const std::string& key) const {
        return has_property(key) ? properties.at(key) : "";
    }
    void read(rapidxml::xml_node<>& parent_node);
    void save(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node);
private:
    std::unordered_map<std::string, std::string> properties;
};

#endif
