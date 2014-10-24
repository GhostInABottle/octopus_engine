#ifndef HPP_OBJECT_LAYER
#define HPP_OBJECT_LAYER

#include <memory>
#include <vector>
#include <xd/graphics/types.hpp>
#include "rapidxml.hpp"
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

    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node, Game& game, const Camera& camera, Map& map);
};

#endif
