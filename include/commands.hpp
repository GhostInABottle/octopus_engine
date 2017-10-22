#ifndef HPP_COMMANDS
#define HPP_COMMANDS

#include "commands/move_object_command.hpp"
#include "commands/move_object_to_command.hpp"
#include "commands/show_pose_command.hpp"
#include "commands/move_camera_command.hpp"
#include "commands/tint_screen_command.hpp"
#include "commands/update_canvas_command.hpp"
#include "commands/update_layer_command.hpp"
#include "commands/fade_music_command.hpp"
#include "commands/shake_screen_command.hpp"
#include "commands/show_text_command.hpp"
#include "commands/wait_command.hpp"

// Used to simplify NPC scheduling
class Dummy_Command : public Command {
    void execute() {}
    bool is_complete() const { return true; }
    bool is_complete(int ticks) const { return true; }
};


#endif
