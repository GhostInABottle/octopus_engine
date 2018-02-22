#include "../include/sprite_holder.hpp"
#include "../include/direction_utilities.hpp"
#include "../include/sprite.hpp"
#include "../include/game.hpp"

void Sprite_Holder::set_pose(const std::string& pose_name,
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

void Sprite_Holder::reset() {
    if (get_sprite())
        get_sprite()->reset();
}
