#include "../../include/commands/show_pose_command.hpp"
#include "../../include/interfaces/sprite_holder.hpp"
#include "../../include/sprite.hpp"
#include "../../include/sprite_data.hpp"
#include "../../include/map.hpp"
#include "../../include/map_object.hpp"
#include "../../include/layers/image_layer.hpp"
#include "../../include/canvas/sprite_canvas.hpp"
#include <exception>

Sprite_Holder* Show_Pose_Command::get_holder() {
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
        Direction dir) : holder_info(holder_info), complete(false) {
    map_ptr = &map;
    auto holder = get_holder();
    if (!holder) {
        throw std::runtime_error("Sprite holder with id (" + std::to_string(holder_info.id)
            + (holder_info.parent_id != -1 ? ", " + std::to_string(holder_info.parent_id) : std::string{})
            + ") not found when trying to show pose : " + pose_name);
    }

    holder->set_pose(pose_name, state, dir);
}

void Show_Pose_Command::execute() {
    auto holder = get_holder();
    if (!holder) return;

    auto sprite = holder->get_sprite();
    auto& pose = sprite->get_pose();
    auto infinite_pose_complete = pose.repeats == -1 &&
        (!pose.require_completion || sprite->is_completed());
    complete = stopped || infinite_pose_complete || sprite->is_stopped();
}

bool Show_Pose_Command::is_complete() const {
    return complete || force_stopped;
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
