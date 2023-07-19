#ifndef HPP_OBJECT_LAYER
#define HPP_OBJECT_LAYER

#include <memory>
#include <vector>
#include "xd/graphics/types.hpp"
#include "layer.hpp"
#include "interfaces/color_holder.hpp"

class Game;
class Camera;
class Map;
class Map_Object;

struct Object_Layer : public Layer, public Color_Holder {
    // Color multiplied by object colors when rendering objects
    xd::vec4 tint_color;
    // List of objects
    std::vector<Map_Object*> objects;

    xd::vec4 get_color() const override {
        return tint_color;
    }
    void set_color(xd::vec4 new_color) override {
        tint_color = new_color;
    }

    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) override;
    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node, Game& game, const Camera& camera, Map& map);
};

#endif
