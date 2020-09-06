#include "../include/image_layer.hpp"
#include "../include/image_layer_renderer.hpp"
#include "../include/image_layer_updater.hpp"
#include "../include/game.hpp"
#include "../include/utility/color.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/math.hpp"
#include "../include/utility/xml.hpp"
#include "../include/utility/string.hpp"
#include "../include/exceptions.hpp"
#include "../include/sprite_data.hpp"
#include "../include/log.hpp"
#include "../include/configurations.hpp"
#include "../include/xd/system.hpp"

void Image_Layer::set_sprite(Game& game, const std::string& filename,
        const std::string& pose_name) {
    if (!file_utilities::file_exists(filename)) {
        LOGGER_W << "Tried to set sprite for layer " << name <<
                    " to nonexistent file " << filename;
        return;
    }
    sprite = std::make_unique<Sprite>(game, Sprite_Data::load(game.get_asset_manager(), filename));
    set_pose(pose_name, "", Direction::NONE);
}

void Image_Layer::set_image(const std::string& filename) {
    if (!file_utilities::file_exists(filename)) {
        LOGGER_W << "Tried to set image for layer " << name <<
                    " to nonexistent file " << filename;
        return;
    }
    image_source = filename;
    file_utilities::normalize_slashes(image_source);
    image_texture = std::make_shared<xd::texture>(
        image_source, image_trans_color, GL_REPEAT, GL_REPEAT,
        GL_NEAREST, GL_NEAREST);
}

rapidxml::xml_node<>* Image_Layer::save(rapidxml::xml_document<>& doc) {
    auto node = Layer::save(doc, "imagelayer");
    if (!image_source.empty()) {
        auto image_node = xml_node(doc, "image");
        image_node->append_attribute(xml_attribute(doc, "source", image_source));
        if (image_trans_color.a > 0.0f) {
            std::string hex_color = color_to_hex(image_trans_color);
            image_node->append_attribute(xml_attribute(doc, "trans", hex_color));
        }
        node->append_node(image_node);
    }
    return node;
}

std::unique_ptr<Layer> Image_Layer::load(rapidxml::xml_node<>& node, Game& game, const Camera& camera) {
    auto layer_ptr = std::make_unique<Image_Layer>();
    layer_ptr->Layer::load(node);

    // Layer properties
    auto& properties = layer_ptr->properties;
    std::string sprite;
    if (properties.has_property("xspeed"))
        layer_ptr->velocity.x = std::stof(properties["xspeed"]) * 60.0f / Configurations::get<int>("debug.logic-fps");
    if (properties.has_property("yspeed"))
        layer_ptr->velocity.y = std::stof(properties["yspeed"]) * 60.0f / Configurations::get<int>("debug.logic-fps");
    if (properties.has_property("fixed"))
        layer_ptr->fixed = string_utilities::string_to_bool(properties["fixed"]);
    if (properties.has_property("sprite"))
        sprite = properties["sprite"];
    std::string pose;
    if (properties.has_property("pose"))
        pose = properties["pose"];

    // Image
    if (!sprite.empty()) {
        layer_ptr->set_sprite(game, sprite, pose);
    } else if (auto image_node = node.first_node("image")) {
        layer_ptr->image_source = image_node->first_attribute("source")->value();
        file_utilities::normalize_slashes(layer_ptr->image_source);
        if (auto trans_attr = image_node->first_attribute("trans")) {
            layer_ptr->image_trans_color = hex_to_color(trans_attr->value());
        }
        // Load the texture
        layer_ptr->image_texture = std::make_shared<xd::texture>(
            layer_ptr->image_source, layer_ptr->image_trans_color,
            GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    } else {
        throw tmx_exception("Missing image in image layer");
    }

    layer_ptr->renderer = std::make_unique<Image_Layer_Renderer>(*layer_ptr, camera);
    if (!check_close(layer_ptr->velocity.x, 0.0f) || !check_close(layer_ptr->velocity.y, 0.0f))
        layer_ptr->repeat = true;

    if (layer_ptr->sprite || layer_ptr->repeat)
        layer_ptr->updater = std::make_unique<Image_Layer_Updater>(*layer_ptr);

    return layer_ptr;
}
