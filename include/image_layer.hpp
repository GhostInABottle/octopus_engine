#ifndef HPP_IMAGE_LAYER
#define HPP_IMAGE_LAYER

#include <memory>
#include <string>
#include "xd/graphics/texture.hpp"
#include "xd/graphics/types.hpp"
#include "interfaces/sprite_holder.hpp"
#include "interfaces/color_holder.hpp"
#include "sprite.hpp"
#include "layer.hpp"

class Game;
class Camera;

struct Image_Layer : public Layer, public Sprite_Holder, public Color_Holder {
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
    // Color applied to image
    xd::vec4 image_color;
    // Image texture
    std::shared_ptr<xd::texture> image_texture;
    // Optional sprite
    std::unique_ptr<Sprite> sprite;

    // Constructor
    Image_Layer() noexcept : repeat(false), fixed(false), image_color(1.0f) {}
    // Set the sprite
    using Sprite_Holder::set_sprite;
    void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") override;
    // Set image
    void set_image(const std::string& filename);
    // Get the sprite, if any
    Sprite* get_sprite() noexcept override { return sprite.get(); }
    // Get the image tint color (for color holder)
    xd::vec4 get_color() const override {
        return image_color;
    }
    // Set the image tint color (for color holder)
    void set_color(xd::vec4 new_color) override {
        image_color = new_color;
    }
    // Save as XML
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) override;
    // Load from XML
    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node,
        Game& game, const Camera& camera);
};

#endif
