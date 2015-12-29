#include "../include/tileset.hpp"
#include "../include/utility.hpp"
#include "../include/exceptions.hpp"
#include <boost/lexical_cast.hpp>
#include <xd/system.hpp>
#include <xd/factory.hpp>

rapidxml::xml_node<>* Tileset::save(rapidxml::xml_document<>& doc) {
    auto node = xml_node(doc, "tileset");
    node->append_attribute(xml_attribute(doc, "firstgid", std::to_string(first_id)));
    node->append_attribute(xml_attribute(doc, "name", name));
    node->append_attribute(xml_attribute(doc, "tilewidth", std::to_string(tile_width)));
    node->append_attribute(xml_attribute(doc, "tileheight", std::to_string(tile_height)));
    if (!filename.empty())
        node->append_attribute(xml_attribute(doc, "source", filename));
    // Tileset properties
    save_properties(properties, doc, *node);
    // Image
    if (!image_source.empty()) {
        auto image_node = xml_node(doc, "image");
        image_node->append_attribute(xml_attribute(doc, "source", image_source));
        if (image_trans_color.a > 0.0f) {
            std::string hex_color = color_to_hex(image_trans_color);
            image_node->append_attribute(xml_attribute(doc, "trans", hex_color));
        }
        node->append_node(image_node);
    }
    // Tiles
    for (auto& tile : tiles) {
        auto tile_node = xml_node(doc, "tile");
        tile_node->append_attribute(xml_attribute(doc, "id", std::to_string(tile.id)));
        save_properties(tile.properties, doc, *tile_node);
        node->append_node(tile_node);
    }
    return node;
}

std::unique_ptr<Tileset> Tileset::load(const std::string& filename) {
    rapidxml::memory_pool<> pool;
    char* content = pool.allocate_string(read_file(filename).c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(content);
    auto tileset_node = doc.first_node("tileset");
    if (!tileset_node)
        throw tmx_exception("Invalid external tileset TMX file. Missing tileset node");
    auto tileset = load(*tileset_node);
    tileset->filename = normalize_slashes(filename);
    return tileset;
}

std::unique_ptr<Tileset> Tileset::load(rapidxml::xml_node<>& node) {
    using boost::lexical_cast;
    std::unique_ptr<Tileset> tileset_ptr(new Tileset());
	int first_id = 1;
	if (auto first_id_attr = node.first_attribute("firstgid"))
		first_id = lexical_cast<int>(first_id_attr->value());
	tileset_ptr->first_id = first_id;
    tileset_ptr->name = node.first_attribute("name")->value();
    tileset_ptr->tile_width = lexical_cast<int>(
        node.first_attribute("tilewidth")->value());
    tileset_ptr->tile_height = lexical_cast<int>(
        node.first_attribute("tileheight")->value());
    // Tileset properties
    read_properties(tileset_ptr->properties, node);
    // Image
    if (auto image_node = node.first_node("image")) {
        tileset_ptr->image_source = image_node->first_attribute("source")->value();
        if (auto trans_attr = image_node->first_attribute("trans")) {
            tileset_ptr->image_trans_color = hex_to_color(trans_attr->value());
        }
        // Load the texture
        tileset_ptr->image_texture = xd::create<xd::texture>(
            normalize_slashes(tileset_ptr->image_source), 
            tileset_ptr->image_trans_color, GL_REPEAT, GL_REPEAT,
            GL_NEAREST, GL_NEAREST);
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
