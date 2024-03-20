#ifndef HPP_LAYER
#define HPP_LAYER

#include "../interfaces/editable.hpp"
#include "../interfaces/opacity_holder.hpp"
#include "../tmx_properties.hpp"
#include "../vendor/rapidxml.hpp"
#include "../xd/glm.hpp"
#include <memory>
#include <string>

class Layer_Renderer;
class Layer_Updater;

struct Layer : public Editable, public Tmx_Object, public Opacity_Holder {
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
    // Get and set opacity (for the translucent interface)
    float get_opacity() const override { return opacity; }
    void set_opacity(float new_opacity) override { opacity = new_opacity;  }
    // Serialize layer to XML node
    virtual rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) = 0;
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc,
            const std::string& node_name);
    // Load layer data from XML node
    void load(rapidxml::xml_node<>& node);
    virtual ~Layer() = 0;

};

#endif
