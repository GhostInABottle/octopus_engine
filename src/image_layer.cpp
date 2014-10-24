#include "../include/image_layer.hpp"
#include "../include/image_layer_renderer.hpp"
#include "../include/image_layer_updater.hpp"
#include "../include/utility.hpp"
#include "../include/exceptions.hpp"
#include "../include/sprite_data.hpp"
#include <boost/lexical_cast.hpp>
#include <xd/system.hpp>
#include <xd/factory.hpp>

void Image_Layer::set_sprite(Game& game, xd::asset_manager& manager,
        const std::string& filename, const std::string& pose_name) {
    sprite = xd::create<Sprite>(game, Sprite_Data::load(manager, filename));
    set_pose(pose_name, "", Direction::NONE);
}

std::unique_ptr<Layer> Image_Layer::load(rapidxml::xml_node<>& node, Game& game,
        const Camera& camera, xd::asset_manager& manager) {
    using boost::lexical_cast;
    Image_Layer* layer_ptr = new Image_Layer();
    layer_ptr->name = node.first_attribute("name")->value();
    if (auto opacity_node = node.first_attribute("opacity"))
        layer_ptr->opacity = lexical_cast<float>(opacity_node->value());
    if (auto visible_node = node.first_attribute("visible"))
        layer_ptr->visible = lexical_cast<bool>(visible_node->value());

    // Layer properties
    Properties& properties = layer_ptr->properties;
    read_properties(properties, node);

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
        layer_ptr->image_source = filename;
        bool has_trans = false;
        if (auto trans_attr = image_node->first_attribute("trans")) {
            layer_ptr->image_trans_color = hex_to_color(trans_attr->value());
            has_trans = true;
        }
        // Load the image and set color key if needed
        xd::image image(normalize_slashes(layer_ptr->image_source));
        if (has_trans)
            set_color_key(image, layer_ptr->image_trans_color);
        // Load the texture
        layer_ptr->image_texture = xd::create<xd::texture>(image, 
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
