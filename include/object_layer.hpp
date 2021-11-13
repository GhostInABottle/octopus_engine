#ifndef HPP_OBJECT_LAYER
#define HPP_OBJECT_LAYER

#include <memory>
#include <vector>
#include "xd/graphics/types.hpp"
#include "layer.hpp"

class Game;
class Camera;
class Map;
class Map_Object;

struct Object_Layer : public Layer {
    // Color multiplied by object colors when rendering objects
    xd::vec4 tint_color;
    // List of objects
    std::vector<Map_Object*> objects;

    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) override;
    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node, Game& game, const Camera& camera, Map& map);
};

#endif
