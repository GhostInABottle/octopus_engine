#ifndef HPP_LAYER
#define HPP_LAYER

#include<memory>
#include <string>
#include "xd/graphics/types.hpp"
#include "tmx_properties.hpp"
#include "editable.hpp"
#include "vendor/rapidxml.hpp"

class Layer_Renderer;
class Layer_Updater;

struct Layer : public Editable {
    // Layer ID
    int id;
    // Layer name
    std::string name;
    // Layer width (same as map width)
    int width;
    // Layer height (same as map height)
    int height;
    // Opacity value
    float opacity;
    // Is the layer visible?
    bool visible;
    // Layer properties
    Tmx_Properties properties;
    // Layer rendering component
    std::unique_ptr<Layer_Renderer> renderer;
    // Layer logic component
    std::unique_ptr<Layer_Updater> updater;
    // Filename of vertex shader
    std::string vertex_shader;
    // Filename of fragment shader
    std::string fragment_shader;

    Layer();
    // Resize layer (width and height in tiles)
    virtual void resize(xd::ivec2 new_size);
    // Serialize layer to XML node
    virtual rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) = 0;
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc,
            const std::string& node_name);
    // Load layer data from XML node
    void load(rapidxml::xml_node<>& node);
    // Get and set a property
    void set_property(const std::string& name, const std::string& value) {
        properties[name] = value;
    }
    std::string get_property(const std::string& name) const {
        return properties[name];
    }
    virtual ~Layer() = 0;

};

#endif
