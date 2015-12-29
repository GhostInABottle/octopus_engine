#ifndef HPP_SPRITE_DATA
#define HPP_SPRITE_DATA

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <xd/graphics/types.hpp>
#include <xd/graphics/texture.hpp>
#include "rapidxml.hpp"

namespace xd {
    class asset_manager;
}

struct Frame {
    // Frame duration in milliseconds
    int duration;
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
    xd::texture::ptr image;
    // Transparent color
    xd::vec4 transparent_color;
    // Frame sound effect filename
    std::string sound_file;

    Frame() : duration(-1), magnification(1.0f, 1.0f), angle(0),
        opacity(1.0f), tween_frame(false) {}
};

struct Pose {
    // Collision bounding box
    xd::rect bounding_box;
    // Total duration of one pose cycle in milliseconds
    int duration;
    // Number of times pose is repeated (-1 = forever).
    int repeats;
    // Transform origin point
    xd::vec2 origin;
    // Pose image
    xd::texture::ptr image;
    // Transparent color
    xd::vec4 transparent_color;
    // Hash table of tags
    std::unordered_map<std::string, std::string> tags;
    // List of frames
    std::vector<Frame> frames;
    
    Pose() : duration(100), repeats(-1) {}
};

struct Sprite_Data {
    // Sprite file name
    std::string filename;
    // Sprite image
    xd::texture::ptr image;
    // Transparent color
    xd::vec4 transparent_color;
    // List of poses
    std::vector<Pose> poses;
    // Asset manager
    xd::asset_manager& asset_manager;

    Sprite_Data(xd::asset_manager& manager);
    ~Sprite_Data();

    static std::unique_ptr<Sprite_Data> load(xd::asset_manager& manager, const std::string& filename);
    static std::unique_ptr<Sprite_Data> load(xd::asset_manager& manager, rapidxml::xml_node<>& node);
};

#endif
