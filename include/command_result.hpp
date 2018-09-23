#ifndef HPP_COMMAND_RESULT
#define HPP_COMMAND_RESULT

#include "command.hpp"
#include <memory>

class Command_Result {
public:
    Command_Result(std::shared_ptr<Command> command) : command(command) {}
    bool operator()() const { return is_complete(); }
    bool is_complete() const { return command->is_complete(); }
    bool is_complete(int ticks) const { return command->is_complete(ticks); }
    void execute() { command->execute(); }
    void execute(int ticks) { command->execute(ticks); }
	void stop() { command->stop(); }
protected:
    std::shared_ptr<Command> command;
};

class Choice_Result : public Command_Result {
public:
    Choice_Result(std::shared_ptr<Command> command) : Command_Result(command) {}
    int choice_index() const;
};

#endif
