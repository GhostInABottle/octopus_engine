#include "show_pose_command.hpp"
#include "../interfaces/sprite_holder.hpp"
#include "../sprite.hpp"
#include "../sprite_data.hpp"
#include "../map/map.hpp"
#include "../map/map_object.hpp"
#include "../map/layers/image_layer.hpp"
#include "../canvas/sprite_canvas.hpp"
#include <stdexcept>

Sprite_Holder* Show_Pose_Command::get_holder() const {
    auto& map = *map_ptr;
    if (holder_info.type == Holder_Type::MAP_OBJECT) {
        return map.get_object(holder_info.id);
    } else if (holder_info.type == Holder_Type::LAYER) {
        return map.get_image_layer_by_id(holder_info.id);
    } else if (holder_info.type == Holder_Type::CANVAS) {
        auto has_parent = holder_info.parent_id != -1;
        auto id = has_parent ? holder_info.parent_id : holder_info.id;
        auto canvas = map.get_canvas(id);
        if (canvas && has_parent) {
            canvas = canvas->get_child_by_id(holder_info.id);
        }
        return dynamic_cast<Sprite_Canvas*>(canvas);
    } else {
        throw std::runtime_error("Unknown sprite holder type");
    }
}

Show_Pose_Command::Show_Pose_Command(Map& map, Holder_Info holder_info,
        const std::string& pose_name, const std::string& state,
        Direction dir, bool reset_current_frame)
        : holder_info(holder_info)
        , complete(false) {
    map_ptr = &map;
    auto holder = get_holder();
    if (!holder) {
        throw std::runtime_error("Sprite holder with id (" + std::to_string(holder_info.id)
            + (holder_info.parent_id != -1 ? ", " + std::to_string(holder_info.parent_id) : std::string{})
            + ") not found when trying to show pose : " + pose_name);
    }

    holder->set_pose(pose_name, state, dir, reset_current_frame);
}

void Show_Pose_Command::execute() {
    if (stopped) return;

    auto holder = get_holder();
    if (!holder) return;

    auto sprite = holder->get_sprite();
    if (!sprite) return;

    auto& pose = sprite->get_pose();
    auto infinite_pose_complete = pose.repeats == -1 && !pose.require_completion;

    complete = infinite_pose_complete || sprite->is_complete();
}

bool Show_Pose_Command::is_complete() const {
    return complete || stopped;
}

void Show_Pose_Command::pause() {
    Command::pause();

    auto holder = get_holder();
    if (!holder) return;
    holder->get_sprite()->pause();
}

void Show_Pose_Command::resume() {
    Command::resume();

    auto holder = get_holder();
    if (!holder) return;
    holder->get_sprite()->resume();
}
