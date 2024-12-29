#ifndef HPP_SPRITE_DATA
#define HPP_SPRITE_DATA

#include "direction.hpp"
#include "vendor/rapidxml.hpp"
#include "xd/audio/channel_group_type.hpp"
#include "xd/graphics/texture.hpp"
#include "xd/graphics/types.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace xd {
    class asset_manager;
    class audio;
    class sound;
}

struct Frame {
    // Frame duration in milliseconds
    int duration;
    // If specified, a random value between duration and max will be used
    int max_duration;
    // A name used to identify the frame
    std::string marker;
    // Source rectangle
    xd::rect rectangle;
    // X and Y magnification
    xd::vec2 magnification;
    // Rotation angle in degrees
    int angle;
    // Alpha value (0 to 1)
    float opacity;
    // Is it an animation tween frame?
    bool tween_frame;
    // Frame image
    std::shared_ptr<xd::texture> image;
    // Transparent color
    xd::vec4 transparent_color;
    // Frame sound effect
    std::shared_ptr<xd::sound> sound_file;
    // Original sound file volume (before attenuation)
    float sound_volume;

    Frame() noexcept : duration(-1), max_duration(-1), magnification(1.0f, 1.0f),
        angle(0), opacity(1.0f), tween_frame(false), sound_volume(1.0f) {}
};

struct Pose {
    // Collision bounding box
    xd::rect bounding_box;
    // Optional collision circle
    std::optional<xd::circle> bounding_circle;
    // Total duration of one pose cycle in milliseconds
    int duration;
    // Number of times pose is repeated (-1 = forever)
    int repeats;
    // Should infinite pose frames complete before marking as finished?
    bool require_completion;
    // The frames on which an infinite pose is marked as completed
    std::optional<std::vector<int>> completion_frames;
    // Transform origin point
    xd::vec2 origin;
    // Pose image
    std::shared_ptr<xd::texture> image;
    // Transparent color
    xd::vec4 transparent_color;
    // Name for specifying the pose
    std::string name;
    // State (e.g. walk or face) when this pose is picked
    std::string state;
    // Direction represented by this pose
    Direction direction;
    // List of frames
    std::vector<Frame> frames;

    Pose() noexcept : duration(100), repeats(-1), require_completion(false), direction(Direction::NONE) {}
};

struct Sprite_Data {
    // Sprite file name
    std::string filename;
    // Sprite texture
    std::shared_ptr<xd::texture> image;
    // Transparent color for image
    xd::vec4 transparent_color;
    // Default pose when no pose is specified or matches
    std::string default_pose;
    // List of poses
    std::vector<Pose> poses;
    // Does it have any diagonal directions?
    bool has_diagonal_directions;

    explicit Sprite_Data(const std::string& filename);

    static std::shared_ptr<Sprite_Data> load(std::string filename, xd::asset_manager& manager,
        xd::audio* audio, channel_group_type channel_group);
    static std::shared_ptr<Sprite_Data> load(rapidxml::xml_node<>& node, const std::string& filename,
        xd::asset_manager& manager, xd::audio* audio, channel_group_type channel_group);
};

#endif
