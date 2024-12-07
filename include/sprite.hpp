#ifndef HPP_SPRITE
#define HPP_SPRITE

#include "direction.hpp"
#include "xd/entity.hpp"
#include "xd/graphics/sprite_batch.hpp"
#include "xd/graphics/types.hpp"
#include <memory>
#include <optional>
#include <string>

class Game;
struct Frame;
struct Pose;
struct Sprite_Data;
class Map_Object;
class Sprite_Canvas;
class Image_Layer;

class Sprite : public xd::component<Map_Object> {
public:
    Sprite(Game& game, std::shared_ptr<Sprite_Data> data);
    ~Sprite();
    // Render a frame
    void render(Map_Object& object);
    void render(xd::sprite_batch& batch, const Sprite_Canvas& canvas, const xd::vec2 pos) const;
    void render(xd::sprite_batch& batch, const Image_Layer& image_layer, const xd::vec2 pos) const;
    // Frame update
    void update(Map_Object& object);
    void update();
    // Reset values to their defaults
    void reset();
    // Get sprite file name
    std::string get_filename() const;
    // Sets the current pose
    void set_pose(const std::string& pose_name, const std::string& state_name, Direction dir);
    // Get the current pose
    Pose& get_pose();
    const Pose& get_pose() const;
    // Get the default pose name
    std::string get_default_pose() const;
    // Get bounding box
    xd::rect get_bounding_box() const;
    // Get bounding circle, if any
    std::optional<xd::circle> get_bounding_circle() const;
    // Get size of first frame
    xd::vec2 get_size() const;
    // Get current frame
    Frame& get_frame();
    const Frame& get_frame() const;
    // Get current frame index
    int get_frame_index() const;
    // Is the current animation done?
    bool is_complete() const;
    // Wait for pose to reach a specific frame index before marking it as complete
    void complete_at(int frame_index);
    // Wait for the pose to reach the final frame and mark it as complete
    void complete();
    // Is the sprite paused / not updating
    bool is_paused() const;
    // Stop updating the sprite
    void pause();
    // Resume updating the sprite
    void resume();
    // Gets animation speed modifier;
    float get_speed() const;
    // Sets animation speed modifier
    void set_speed(float speed);
    // Does the sprite support diagonal directions?
    bool is_eight_directional() const;
    // Get the volume of the sprite's sound effects (0 to 1)
    float get_sfx_volume() const;
    // Set the volume of the sprite's sound effects (0 to 1)
    void set_sfx_volume(float volume);
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif
