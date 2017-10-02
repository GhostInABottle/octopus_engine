#include "../../include/commands/show_pose_command.hpp"
#include "../../include/sprite_holder.hpp"
#include "../../include/sprite.hpp"
#include "../../include/sprite_data.hpp"



Show_Pose_Command::Show_Pose_Command(Sprite_Holder* holder,
	const std::string& pose_name, const std::string& state,
	Direction dir) : holder(holder) {
	holder->set_pose(pose_name, state, dir);
}

bool Show_Pose_Command::is_complete() const {
	auto sprite = holder->get_sprite();
	return stopped || sprite->get_pose().repeats == -1 || sprite->is_stopped();
}
