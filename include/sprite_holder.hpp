#ifndef HPP_SPRITE_HOLDER
#define HPP_SPRITE_HOLDER

#include <string>
#include <unordered_map>
#include "direction.hpp"

class Game;
class Sprite;

struct Sprite_Holder {
    virtual void set_pose(const std::string& pose_name,
            const std::string& state, Direction direction);
    virtual void reset();
    // Set sprite
    virtual void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") = 0;
    virtual Sprite* get_sprite() = 0;
    std::string get_pose_tag(const std::string& tag);
};

#endif
