#include "../../include/commands/show_pose_command.hpp"
#include "../../include/sprite_holder.hpp"
#include "../../include/sprite.hpp"
#include "../../include/sprite_data.hpp"

Show_Pose_Command::Show_Pose_Command(Map& map, Sprite_Holder* holder,
        const std::string& pose_name, const std::string& state,
        Direction dir) : holder(holder), complete(false) {
    holder->set_pose(pose_name, state, dir);
    map_ptr = &map;
}

void Show_Pose_Command::execute() {
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
    holder->get_sprite()->pause();
}

void Show_Pose_Command::resume() {
    Command::resume();
    holder->get_sprite()->resume();
}