#ifndef HPP_TMX_PROPERTIES
#define HPP_TMX_PROPERTIES

#include "../vendor/rapidxml.hpp"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

class Tmx_Properties {
public:
    bool contains(const std::string& key) const {
        return properties.find(key) != properties.end();
    }
    std::string& operator[](const std::string& key) {
        if (!contains(key)) {
            ordered_keys.push_back(key);
        }
        return properties[key];
    }
    std::string operator[](const std::string& key) const {
        return contains(key) ? properties.at(key) : "";
    }
    void erase(const std::string& key) {
        ordered_keys.erase(std::remove(ordered_keys.begin(),
            ordered_keys.end(), key), ordered_keys.end());
        properties.erase(key);
    }
    void read(rapidxml::xml_node<>& parent_node);
    void save(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node);
private:
    std::unordered_map<std::string, std::string> properties;
    std::vector<std::string> ordered_keys;
};

struct Tmx_Object {
    void set_property(const std::string& name, const std::string& value) {
        properties[name] = value;
    }
    std::string get_property(const std::string& name) const {
        return properties[name];
    }
    bool has_property(const std::string& name) const {
        return properties.contains(name);
    }
    void erase_property(const std::string& name) {
        properties.erase(name);
    }
    void set_editor_property(const std::string& prop_name, const std::string& value,
        const std::string& default_value = "", bool capitalize_value = true);
    Tmx_Properties properties;
};

#endif
