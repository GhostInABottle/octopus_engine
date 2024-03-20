#include "../../include/direction.hpp"
#include "../../include/interfaces/sprite_holder.hpp"
#include "../../include/sprite.hpp"

Sprite_Holder::~Sprite_Holder() {}

void Sprite_Holder::set_pose(const std::string& pose_name,
        const std::string& state, Direction direction) {
    auto sprite = get_sprite();
    if (sprite) {
        sprite->set_pose(pose_name, state, direction);
    }
}

void Sprite_Holder::reset() {
    auto sprite = get_sprite();
    if (sprite) sprite->reset();
}

std::string Sprite_Holder::get_sprite_filename() {
    auto sprite = get_sprite();
    if (!sprite) return "";
    return sprite->get_filename();
}