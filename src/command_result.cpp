#include "../include/command_result.hpp"
#include "../include/commands.hpp"

int Choice_Result::choice_index() const {
    Text_Command* text_command = dynamic_cast<Text_Command*>(command.get());
    return text_command->choice_index();
}
