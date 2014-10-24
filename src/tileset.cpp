#include "../include/tileset.hpp"
#include "../include/utility.hpp"
#include "../include/exceptions.hpp"
#include <boost/lexical_cast.hpp>
#include <xd/system.hpp>
#include <xd/factory.hpp>

std::unique_ptr<Tileset> Tileset::load(const std::string& filename) {
    rapidxml::memory_pool<> pool;
    char* content = pool.allocate_string(read_file(filename).c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(content);
    auto tileset_node = doc.first_node("tileset");
    if (!tileset_node)
        throw tmx_exception("Invalid external tileset TMX file. Missing tileset node");
    return load(*tileset_node);
}

std::unique_ptr<Tileset> Tileset::load(rapidxml::xml_node<>& node) {
    using boost::lexical_cast;
    std::unique_ptr<Tileset> tileset_ptr(new Tileset());
    auto first_id_node = node.first_attribute("firstgid");
    if (first_id_node)
        tileset_ptr->first_id = lexical_cast<int>(first_id_node->value());

    tileset_ptr->name = node.first_attribute("name")->value();
    tileset_ptr->tile_width = lexical_cast<int>(node.first_attribute("tilewidth")->value());
    tileset_ptr->tile_height = lexical_cast<int>(node.first_attribute("tileheight")->value());

    // Tileset properties
    read_properties(tileset_ptr->properties, node);

    // Image
    bool has_trans = false;
    if (auto image_node = node.first_node("image")) {
        tileset_ptr->image_source = image_node->first_attribute("source")->value();
        if (auto trans_attr = image_node->first_attribute("trans")) {
            tileset_ptr->image_trans_color = hex_to_color(trans_attr->value());
            has_trans = true;
        }
        // Load the image and set color key if needed
        xd::image image(normalize_slashes(tileset_ptr->image_source));
        if (has_trans)
            set_color_key(image, tileset_ptr->image_trans_color);
        // Load the texture
        tileset_ptr->image_texture = xd::create<xd::texture>(image, 
            GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    }

    // Tiles
    for (auto tile_node = node.first_node("tile"); 
            tile_node; tile_node = tile_node->next_sibling("tile")) {
        Tile tile;
        tile.id = lexical_cast<int>(tile_node->first_attribute("id")->value());
        read_properties(tile.properties, *tile_node);
        tileset_ptr->tiles.push_back(tile);
    }

    return tileset_ptr;
}

xd::rect Tileset::tile_source_rect(int tile_index) const {
    xd::rect src(0, 0, tile_width, tile_height);
    int tileset_width = image_texture->width() / tile_width;
    src.x = static_cast<float>(tile_index % tileset_width) * tile_width;
    src.y = static_cast<float>(tile_index / tileset_width) * tile_height;
    return src;
}
