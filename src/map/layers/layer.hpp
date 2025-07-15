#ifndef HPP_LAYER
#define HPP_LAYER

#include "../../interfaces/editable.hpp"
#include "../../interfaces/opacity_holder.hpp"
#include "../../map/tmx_properties.hpp"
#include "../../vendor/rapidxml.hpp"
#include "../../xd/glm.hpp"
#include <memory>
#include <string>

class Layer_Renderer;
class Layer_Updater;

class Layer : public Editable, public Tmx_Object, public Opacity_Holder {
public:
    Layer();
    // Resize layer (width and height in tiles)
    virtual void resize(xd::ivec2 new_size);
    // Getters and setters
    int get_id() const { return id; }
    std::string get_name() const { return name; }
    void set_name(const std::string& new_name) { name = new_name; }
    int get_width() const { return width; }
    int get_height() const { return height; }
    float get_opacity() const override { return opacity; }
    void set_opacity(float new_opacity) override { opacity = new_opacity; }
    bool is_visible() const { return visible; }
    virtual void set_visible(bool new_visible) {
        visible = new_visible;
    }
    Layer_Renderer* get_renderer() const { return renderer.get(); }
    Layer_Updater* get_updater() const { return updater.get(); }
    bool has_vertex_shader() const { return !vertex_shader.empty(); }
    bool has_fragment_shader() const { return !fragment_shader.empty(); }
    std::string get_vertex_shader() const { return vertex_shader; }
    std::string get_fragment_shader() const { return fragment_shader; }
    // Serialize layer to XML node
    virtual rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) = 0;
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc,
            const std::string& node_name);
    // Load layer data from XML node
    void load(rapidxml::xml_node<>& node);
    virtual ~Layer() = 0;
protected:
    // Layer width (same as map width)
    int width;
    // Layer height (same as map height)
    int height;
    // Layer rendering component
    std::unique_ptr<Layer_Renderer> renderer;
    // Layer logic component
    std::unique_ptr<Layer_Updater> updater;
private:
    // Layer ID
    int id;
    // Layer name
    std::string name;
    // Opacity value
    float opacity;
    // Is the layer visible?
    bool visible;
    // Filename of vertex shader
    std::string vertex_shader;
    // Filename of fragment shader
    std::string fragment_shader;
};

#endif
