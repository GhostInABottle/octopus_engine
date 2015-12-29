#include "../include/image_layer.hpp"
#include "../include/image_layer_renderer.hpp"
#include "../include/image_layer_updater.hpp"
#include "../include/utility.hpp"
#include "../include/exceptions.hpp"
#include "../include/sprite_data.hpp"
#include "../include/log.hpp"
#include <boost/lexical_cast.hpp>
#include <xd/system.hpp>
#include <xd/factory.hpp>

void Image_Layer::set_sprite(Game& game, xd::asset_manager& manager,
        const std::string& filename, const std::string& pose_name) {
    if (!file_exists(filename)) {
        LOGGER_W << "Tried to set sprite for layer " << name <<
                    " to nonexistent file " << filename;
        return;
    }
    sprite = xd::create<Sprite>(game, Sprite_Data::load(manager, filename));
    set_pose(pose_name, "", Direction::NONE);
}

void Image_Layer::set_image(const std::string& filename) {
    if (!file_exists(filename)) {
        LOGGER_W << "Tried to set image for layer " << name <<
                    " to nonexistent file " << filename;
        return;
    }
    image_source = normalize_slashes(filename);
    image_texture = xd::create<xd::texture>(
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

std::unique_ptr<Layer> Image_Layer::load(rapidxml::xml_node<>& node, Game& game,
        const Camera& camera, xd::asset_manager& manager) {
    using boost::lexical_cast;
    Image_Layer* layer_ptr = new Image_Layer();
    layer_ptr->Layer::load(node);

    // Layer properties
    Properties& properties = layer_ptr->properties;
    std::string sprite;
    if (properties.find("xspeed") != properties.end())
        layer_ptr->velocity.x = lexical_cast<float>(properties["xspeed"]);
    if (properties.find("yspeed") != properties.end())
        layer_ptr->velocity.y = lexical_cast<float>(properties["yspeed"]);
    if (properties.find("fixed") != properties.end())
        layer_ptr->fixed = lexical_cast<bool>(properties["fixed"]);
    if (properties.find("sprite") != properties.end())
        sprite = properties["sprite"];
    std::string pose;
    if (properties.find("pose") != properties.end())
        pose = properties["pose"];

    // Image
    if (!sprite.empty()) {
        layer_ptr->set_sprite(game, manager, sprite, pose);
    } else if (auto image_node = node.first_node("image")) {
        std::string filename = image_node->first_attribute("source")->value();
        layer_ptr->image_source = normalize_slashes(filename);
        if (auto trans_attr = image_node->first_attribute("trans")) {
            layer_ptr->image_trans_color = hex_to_color(trans_attr->value());
        }
        // Load the texture
        layer_ptr->image_texture = xd::create<xd::texture>(
            layer_ptr->image_source, layer_ptr->image_trans_color,
            GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    } else {
        throw tmx_exception("Missing image in image layer");
    }

    layer_ptr->renderer.reset(new Image_Layer_Renderer(*layer_ptr, camera));
    if (!check_close(layer_ptr->velocity.x, 0.0f) || !check_close(layer_ptr->velocity.y, 0.0f))
        layer_ptr->repeat = true;

    if (layer_ptr->sprite || layer_ptr->repeat)
        layer_ptr->updater.reset(new Image_Layer_Updater(layer_ptr));

    return std::unique_ptr<Layer>(layer_ptr);
}
