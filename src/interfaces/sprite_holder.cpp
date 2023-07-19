#include "../../include/interfaces/sprite_holder.hpp"
#include "../../include/sprite.hpp"
#include "../../include/game.hpp"
#include "../../include/sprite_data.hpp"
#include "../../include/utility/direction.hpp"
#include "../../include/utility/string.hpp"

Sprite_Holder::~Sprite_Holder() {}

void Sprite_Holder::set_pose(const std::string& pose_name,
        const std::string& state, Direction direction) {
    if (get_sprite()) {
        get_sprite()->set_pose(pose_name, state, direction);
    }
}

void Sprite_Holder::reset() {
    if (get_sprite())
        get_sprite()->reset();
}

std::string Sprite_Holder::get_sprite_filename() {
    auto sprite = get_sprite();
    if (!sprite) return "";
    return sprite->get_filename();
}