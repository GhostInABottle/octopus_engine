#include "../../include/direction.hpp"
#include "../../include/interfaces/sprite_holder.hpp"
#include "../../include/sprite.hpp"

Sprite_Holder::~Sprite_Holder() {}

void Sprite_Holder::set_pose(const std::string& pose_name, const std::string& state,
        Direction direction, bool reset_current_frame) {
    auto sprite = get_sprite();
    if (sprite) {
        sprite->set_pose(pose_name, state, direction, reset_current_frame);
    }
}

void Sprite_Holder::reset() {
    auto sprite = get_sprite();
    if (sprite) {
        sprite->reset();
    }
}

std::string Sprite_Holder::get_sprite_filename() const {
    auto sprite = get_sprite();
    return sprite ? sprite->get_filename() : "";
}

int Sprite_Holder::get_frame_index() const {
    auto sprite = get_sprite();
    return sprite ? sprite->get_frame_index() : 0;
}

std::optional<std::string> Sprite_Holder::get_last_marker() const {
    auto sprite = get_sprite();
    return sprite ? sprite->get_last_marker() : std::nullopt;
}

bool Sprite_Holder::passed_marker(const std::string& marker) const {
    auto sprite = get_sprite();
    return sprite ? sprite->passed_marker(marker) : false;
}
