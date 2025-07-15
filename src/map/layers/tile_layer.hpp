#ifndef HPP_TILE_LAYER
#define HPP_TILE_LAYER

#include <vector>
#include <memory>
#include "layer.hpp"

class Camera;

class Tile_Layer : public Layer {
public:
    // Get the tile list
    std::vector<unsigned int>& get_tiles() { return tiles; }
    const std::vector<unsigned int>& get_tiles() const { return tiles; }
    // Resize the tile layer
    void resize(xd::ivec2 new_size) override;
    // Save and load the tile layer TMX data
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) override;
    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node, Camera& camera);
private:
    // List of tiles
    std::vector<unsigned int> tiles;
};

#endif
