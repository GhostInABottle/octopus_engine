#include "../include/tile_layer.hpp"
#include "../include/tile_layer_renderer.hpp"
#include "../include/utility.hpp"
#include "../include/base64.hpp"
#include "../include/exceptions.hpp"
#include <zlib.h>
#include <boost/lexical_cast.hpp>

std::unique_ptr<Layer> Tile_Layer::load(rapidxml::xml_node<>& node, Camera& camera) {
    using boost::lexical_cast;
    Tile_Layer* layer_ptr = new Tile_Layer();
    layer_ptr->name = node.first_attribute("name")->value();
    layer_ptr->width = lexical_cast<int>(node.first_attribute("width")->value());
    layer_ptr->height = lexical_cast<int>(node.first_attribute("height")->value());
    if (auto opacity_node = node.first_attribute("opacity"))
        layer_ptr->opacity = lexical_cast<float>(opacity_node->value());
    if (auto visible_node = node.first_attribute("visible"))
        layer_ptr->visible = lexical_cast<bool>(visible_node->value());

    // Layer properties
    read_properties(layer_ptr->properties, node);

    // Layer data
    auto data_node = node.first_node("data");
    if (!data_node)
        throw tmx_exception("Missing layer data");
    auto enc_node = data_node->first_attribute("encoding");
    if (!enc_node || enc_node->value() != std::string("base64"))
        throw tmx_exception("Invalid layer data encoding, expected base64");
    auto comp_node = data_node->first_attribute("compression");
    if (!comp_node || comp_node->value() != std::string("zlib"))
        throw tmx_exception("Invalid layer data compression, expected zlib");
    // Get raw base 64 encoded and compressed layer data
    std::string raw_data = trim(data_node->value());
    // Decode layer data
    std::string decoded_data = base64_decode(raw_data);
    // Decompress layer data
    int num_tiles = layer_ptr->width * layer_ptr->height;
    uLongf size = num_tiles * 4;
    const std::unique_ptr<unsigned int> tile_array(new unsigned int[num_tiles]);
    uncompress((Bytef*) tile_array.get(), &size, 
        (const Bytef*) decoded_data.c_str(), decoded_data.size());
    // Put decopressed and decoded  data in the tiles vector
    layer_ptr->tiles = std::vector<unsigned int>(tile_array.get(), tile_array.get() + num_tiles);

    layer_ptr->renderer.reset(new Tile_Layer_Renderer(*layer_ptr, camera));

    return std::unique_ptr<Layer>(layer_ptr);
}
