#ifndef HPP_SPRITE_CANVAS
#define HPP_SPRITE_CANVAS

#include "base_image_canvas.hpp"
#include "../interfaces/sprite_holder.hpp"
#include "../sprite.hpp"

// A canvas for displaying a sprite
class Sprite_Canvas : public Base_Image_Canvas, public Sprite_Holder {
public:
    Sprite_Canvas(const Sprite_Canvas&) = delete;
    Sprite_Canvas& operator=(const Sprite_Canvas&) = delete;
    // Create an image canvas with an optional transparent color
    Sprite_Canvas(Game& game, const std::string& sprite, xd::vec2 position, const std::string& pose_name = "");
    // Change or update the sprite
    void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") override;
    // Get the sprite, if any
    Sprite* get_sprite() override { return sprite.get(); }
    // Get pose name
    std::string get_pose_name();
    // Get pose state
    std::string get_pose_state();
    // Get pose direction
    Direction get_pose_direction();
    // Render the sprite
    void render(Camera& camera, xd::sprite_batch& batch, Base_Canvas* parent) override;
    // Update the sprite
    void update() override { sprite->update(); }
private:
    // The underlying sprite
    std::unique_ptr<Sprite> sprite;
};

#endif