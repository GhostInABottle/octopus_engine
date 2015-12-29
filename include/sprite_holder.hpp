#ifndef HPP_SPRITE_HOLDER
#define HPP_SPRITE_HOLDER

#include <string>
#include <unordered_map>
#include "direction.hpp"

class Game;
class Sprite;
namespace xd {
    class asset_manager;
}

struct Sprite_Holder {
    virtual void set_pose(const std::string& pose_name,
            const std::string& state, Direction direction);
    virtual void reset();
    // Set sprite using default asset manager for this type
    virtual void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "");
    // Set sprite using specified asset manager
    virtual void set_sprite(Game& game, xd::asset_manager& manager, const std::string& filename, const std::string& pose_name = "") = 0;
    virtual Sprite* get_sprite() = 0;
};

#endif
