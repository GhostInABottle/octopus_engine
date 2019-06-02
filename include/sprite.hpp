#ifndef HPP_SPRITE
#define HPP_SPRITE

#include <memory>
#include "xd/system.hpp"
#include "xd/entity.hpp"
#include "xd/graphics/types.hpp"
#include "xd/graphics/sprite_batch.hpp"

class Game;
struct Frame;
struct Pose;
struct Sprite_Data;
class Map_Object;

class Sprite : public xd::component<Map_Object> {
public:
    Sprite(Game& game, std::unique_ptr<Sprite_Data> data);
    // Render a frame
    void render(Map_Object& object);
    void render(xd::sprite_batch& batch, xd::vec2 pos,
        float opacity = 1.0f, xd::vec4 color = xd::vec4(1.0f),
        bool repeat = false, xd::vec2 repeat_pos = xd::vec2());
    // Frame update
    void update(Map_Object& object);
    void update();
    // Reset values to their defaults
    void reset();
    // Get sprite file name
    std::string get_filename() const;
    // Sets the current pose
    void set_pose(const std::unordered_map<std::string, std::string>& new_tags);
    // Get the current pose
    Pose& get_pose();
    // Get bounding box
    xd::rect get_bounding_box() const;
    // Get size of first frame
    xd::vec2 get_size() const;
    // Get current frame
    Frame& get_frame();
    const Frame& get_frame() const;
    // Is the current animation done?
    bool is_stopped() const;
    // Stop updating the sprite
    void stop();
    // Did we reach the last frame?
    bool is_completed() const;
    // Gets animation speed modifier;
    float get_speed() const;
    // Sets animation speed modifier
    void set_speed(float speed);
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif
