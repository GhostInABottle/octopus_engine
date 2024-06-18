#ifndef HPP_IMAGE_LAYER
#define HPP_IMAGE_LAYER

#include "../../interfaces/color_holder.hpp"
#include "../../interfaces/sprite_holder.hpp"
#include "../../sprite.hpp"
#include "../../xd/glm.hpp"
#include "../../xd/graphics/texture.hpp"
#include "layer.hpp"
#include <memory>
#include <string>

class Game;
class Camera;

class Image_Layer : public Layer, public Sprite_Holder, public Color_Holder {
public:
    // Constructor
    Image_Layer() noexcept : repeat(false), fixed(false), image_color(1.0f) {}
    // Set the sprite
    using Sprite_Holder::set_sprite;
    void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") override;
    // Set image
    void set_image(const std::string& filename);
    // Get the sprite, if any
    Sprite* get_sprite() override { return sprite.get(); }
    const Sprite* get_sprite() const override { return sprite.get(); }
    // Getters and setters
    void set_visible(bool new_visible) override {
        if (new_visible && !is_visible() && sprite) {
            sprite->reset();
        }

        Layer::set_visible(new_visible);
    }
    bool is_repeating() const { return repeat; }
    bool is_fixed() const { return fixed; }
    xd::vec2 get_velocity() const {
        return velocity;
    }
    void set_velocity(xd::vec2 new_velocity) {
        velocity = new_velocity;
    }
    xd::vec2 get_position() const { return position; }
    void set_position(xd::vec2 new_position) { position = new_position; }
    std::string get_image_filename() const { return image_source; }
    xd::vec4 get_transparent_color() const { return image_trans_color; }
    xd::vec4 get_color() const override {
        return image_color;
    }
    void set_color(xd::vec4 new_color) override {
        image_color = new_color;
    }
    std::shared_ptr<xd::texture> get_texture() const { return image_texture; }
    // Save as XML
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc) override;
    // Load from XML
    static std::unique_ptr<Layer> load(rapidxml::xml_node<>& node,
        Game& game, const Camera& camera);
private:
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
};

#endif
