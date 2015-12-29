#ifndef HPP_TILE_LAYER
#define HPP_TILE_LAYER

#include <vector>
#include <memory>
#include "layer.hpp"

class Camera;

struct Tile_Layer : public Layer {
    // List of tiles
    std::vector<unsigned int> tiles;

    void resize(xd::ivec2 new_size);
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc);
    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node, Camera& camera);
};

#endif
