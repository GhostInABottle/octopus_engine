#ifndef HPP_OBJECT_LAYER
#define HPP_OBJECT_LAYER

#include <memory>
#include <vector>
#include <xd/graphics/types.hpp>
#include "layer.hpp"

class Game;
class Camera;
class Map;
class Map_Object;

struct Object_Layer : public Layer {
    // Display color for objects
    xd::vec4 color;
    // List of objects
    std::vector<Map_Object*> objects;

    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc);
    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node, Game& game, const Camera& camera, Map& map);
};

#endif
