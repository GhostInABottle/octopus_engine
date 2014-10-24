#ifndef HPP_SPRITE_HOLDER
#define HPP_SPRITE_HOLDER

#include <string>
#include <unordered_map>
#include "direction.hpp"
#include "direction_utilities.hpp"
#include "sprite.hpp"

class Game;
namespace xd {
    class asset_manager;
}

struct Sprite_Holder {
    virtual void set_pose(const std::string& pose_name,
            const std::string& state, Direction direction) {
        if (get_sprite()) {
            std::unordered_map<std::string, std::string> tags;
            if (!pose_name.empty())
                tags["name"] = pose_name;
            if (!state.empty())
                tags["state"] = state;
            if (direction != Direction::NONE)
                tags["direction"] = direction_to_string(direction);
            get_sprite()->set_pose(tags);
        }
    }
    virtual void reset() {
        if (get_sprite())
            get_sprite()->reset();
    }
    virtual void set_sprite(Game& game, xd::asset_manager& manager, const std::string& filename, const std::string& pose_name = "") = 0;
    virtual Sprite* get_sprite() = 0;
};

#endif
