#ifndef HPP_TILESET
#define HPP_TILESET

#include <vector>
#include <string>
#include <memory>
#include "xd/graphics/texture.hpp"
#include "xd/graphics/types.hpp"
#include "vendor/rapidxml.hpp"
#include "tmx_properties.hpp"

struct Tileset {
    struct Tile {
        int id;
        Tmx_Properties properties;
    };
    // GID of first tile
    int first_id;
    // Name of tileset
    std::string name;
    // Filename (for external tilesets)
    std::string filename;
    // Width of each tile in pixels
    int tile_width;
    // Height of each tile pixels
    int tile_height;
    // Tileset properties
    Tmx_Properties properties;
    // Image source file
    std::string image_source;
    // Image transparent color
    xd::vec4 image_trans_color;
    // Image texture
    std::shared_ptr<xd::texture> image_texture;
    // List of tiles properties
    std::vector<Tile> tiles;

    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc);
    static std::unique_ptr<Tileset> load(const std::string& filename);
    static std::unique_ptr<Tileset> load(rapidxml::xml_node<>& node);
    std::string set_property(const std::string& name, const std::string& value) {
        properties[name] = value;
    }
    std::string get_property(const std::string& name) const {
        return properties[name];
    }
    xd::rect tile_source_rect(int tile_index) const;
};

#endif
