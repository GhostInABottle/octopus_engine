#include "../../include/exceptions.hpp"
#include "../../include/layers/tile_layer.hpp"
#include "../../include/layers/tile_layer_renderer.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/utility/xml.hpp"
#include "../../include/vendor/base64.hpp"
#include <cmath>
#include <zlib.h>

void Tile_Layer::resize(xd::ivec2 new_size) {
    std::vector<unsigned int> new_tiles;
    auto row_start = tiles.begin();
    int min_width = std::min(new_size.x, this->width);
    int min_height = std::min(new_size.y, this->height);
    for (int y = 0; y < min_height; ++y) {
        auto row_end = row_start + min_width;
        new_tiles.insert(new_tiles.end(), row_start, row_end);
        if (new_size.x > this->width) {
            new_tiles.insert(new_tiles.end(), new_size.x - this->width, 0);
        }
        row_start += this->width;
    }
    if (new_size.y > this->height) {
        int new_rows = (new_size.y - this->height) * new_size.x;
        new_tiles.insert(new_tiles.end(), new_rows, 0);
    }
    tiles = std::move(new_tiles);
    Layer::resize(new_size);
}

rapidxml::xml_node<>* Tile_Layer::save(rapidxml::xml_document<>& doc) {
    auto node = Layer::save(doc, "layer");
    uLongf src_size = tiles.size() * 4;
    uLongf dest_size = compressBound(src_size);
    std::vector<Bytef> dest_bytes(dest_size);
    compress2(&dest_bytes[0], &dest_size,
        (const Bytef*) &tiles[0], src_size,
        Z_DEFAULT_COMPRESSION);
    dest_bytes.resize(dest_size);
    std::string coded_data = base64_encode(
        (const unsigned char*) &dest_bytes[0], dest_bytes.size());
    auto data_node = xml_node(doc, "data", coded_data);
    data_node->append_attribute(xml_attribute(doc, "encoding", "base64"));
    data_node->append_attribute(xml_attribute(doc, "compression", "zlib"));
    node->append_node(data_node);
    return node;
}

std::unique_ptr<Layer> Tile_Layer::load(rapidxml::xml_node<>& node, Camera& camera) {
    auto layer_ptr = std::make_unique<Tile_Layer>();
    layer_ptr->Layer::load(node);

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
    std::string raw_data = data_node->value();
    string_utilities::trim(raw_data);

    // Decode and compress layer data
    std::string decoded_data = base64_decode(raw_data);
    int num_tiles = layer_ptr->width * layer_ptr->height;
    uLongf size = num_tiles * 4;
    const std::unique_ptr<unsigned int> tile_array(new unsigned int[num_tiles]);
    auto result = uncompress((Bytef*) tile_array.get(), &size,
        (const Bytef*) decoded_data.c_str(), decoded_data.size());

    if (result != Z_OK) {
        std::string error;
        switch (result) {
        case Z_MEM_ERROR:
            error = "insufficient memory";
            break;
        case Z_BUF_ERROR:
            error = "insufficient space in output buffer";
            break;
        case Z_DATA_ERROR:
            error = "incomplete or corrupt tile data";
            break;
        default:
            error = "Unknown error " + std::to_string(result);
            break;
        }
        throw tmx_exception("Error while uncompressing tile layer. Error code: " + error);
    }

    // Put decopressed and decoded  data in the tiles vector
    layer_ptr->tiles = std::vector<unsigned int>(tile_array.get(), tile_array.get() + num_tiles);

    layer_ptr->renderer = std::make_unique<Tile_Layer_Renderer>(*layer_ptr, camera);

    return layer_ptr;
}
