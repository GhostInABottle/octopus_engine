#include "../include/sprite_holder.hpp"
#include "../include/sprite.hpp"
#include "../include/game.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility/direction.hpp"
#include "../include/utility/string.hpp"

void Sprite_Holder::set_pose(const std::string& pose_name,
        const std::string& state, Direction direction) {
    if (get_sprite()) {
        std::unordered_map<std::string, std::string> tags;
        if (!pose_name.empty())
            tags["NAME"] = capitalize(pose_name);
        if (!state.empty())
            tags["STATE"] = capitalize(state);
        if (direction != Direction::NONE)
            tags["DIRECTION"] = capitalize(direction_to_string(direction));
        get_sprite()->set_pose(tags);
    }
}

void Sprite_Holder::reset() {
    if (get_sprite())
        get_sprite()->reset();
}

std::string Sprite_Holder::get_pose_tag(const std::string& tag) {
    auto sprite = get_sprite();
    if (!sprite) return "";

    auto& tags = sprite->get_pose().tags;
    if (tags.find(tag) != tags.end())
        return tags[tag];
    else
        return "";
}