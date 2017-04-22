#include "../include/command_result.hpp"
#include "../include/commands/show_text_command.hpp"

int Choice_Result::choice_index() const {
    auto text_command = dynamic_cast<Show_Text_Command*>(command.get());
    return text_command->choice_index();
}
