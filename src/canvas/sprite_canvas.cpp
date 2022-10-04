#include "../../include/canvas/sprite_canvas.hpp"
#include "../../include/sprite_data.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/xd/graphics/sprite_batch.hpp"

Sprite_Canvas::Sprite_Canvas(Game& game, const std::string& sprite, xd::vec2 position, const std::string& pose_name)
        : Base_Image_Canvas(game, Base_Canvas::Type::SPRITE, position, sprite) {
    set_sprite(game, sprite, pose_name);
}

void Sprite_Canvas::set_sprite(Game& game, const std::string& sprite_filename, const std::string& pose_name) {
    if (sprite && filename == sprite_filename)
        return;

    filename = sprite_filename;
    auto& asset_manager = game.get_asset_manager();
    auto audio = game.get_audio();
    auto channel_group = game.get_sound_group_type();
    sprite = std::make_unique<Sprite>(game,
        Sprite_Data::load(sprite_filename, asset_manager, audio, channel_group));
    set_pose(pose_name, "", Direction::NONE);

    redraw();
}

std::string Sprite_Canvas::get_pose_name() {
    return sprite->get_pose().name;
}

std::string Sprite_Canvas::get_pose_state() {
    return sprite->get_pose().state;
}

Direction Sprite_Canvas::get_pose_direction() {
    return sprite->get_pose().direction;
}

void Sprite_Canvas::render(Camera& camera, xd::sprite_batch& batch, Base_Canvas* parent) {
    xd::vec2 pos = get_position();
    if (is_camera_relative()) {
        auto camera_pos = camera.get_pixel_position();
        pos += camera_pos;
    }
    if (parent) {
        pos += parent->get_position();
    }

    sprite->render(batch, *this, pos);
}
