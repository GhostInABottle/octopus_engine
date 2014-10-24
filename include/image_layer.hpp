#ifndef HPP_IMAGE_LAYER
#define HPP_IMAGE_LAYER

#include <memory>
#include <string>
#include <xd/graphics/texture.hpp>
#include <xd/graphics/types.hpp>
#include "sprite_holder.hpp"
#include "rapidxml.hpp"
#include "layer.hpp"

class Game;
class Camera;
namespace xd {
    class asset_manager;
}

struct Image_Layer : public Layer, public Sprite_Holder {
    // Does the layer repeat?
    bool repeat;
    // Is layer fixed or does it move with the camera?
    bool fixed;
    // Layer scroll speed
    xd::vec2 velocity;
    // Layer scroll position
    xd::vec2 position;
    // Image source file
    std::string image_source;
    // Image transparent color
    xd::vec4 image_trans_color;
    // Image texture
    xd::texture::ptr image_texture;
    // Optional sprite
    Sprite::ptr sprite;
    // Constructor
    Image_Layer() : repeat(false), fixed(false) {}
    // Set the sprite
    void set_sprite(Game& game, xd::asset_manager& manager, const std::string& filename, const std::string& pose_name = "");
    // Get the sprite, if any
    Sprite* get_sprite() { return sprite.get(); }

    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node,
        Game& game, const Camera& camera, xd::asset_manager& manager);
};

#endif
