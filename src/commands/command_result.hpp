#ifndef HPP_COMMAND_RESULT
#define HPP_COMMAND_RESULT

#include "command.hpp"
#include <memory>

class Command_Result {
public:
    explicit Command_Result(std::shared_ptr<Command> command) : command(command) {}
    bool operator()() const { return is_complete(); }
    bool is_complete() const { return command->is_complete(); }
    bool is_complete(int ticks) const { return command->is_complete(ticks); }
    void execute() { command->execute(); }
    void execute(int ticks) { command->execute(ticks); }
    void stop() { command->stop(); }
    void force_stop() { command->force_stop(); }
    bool is_stopped() const { return command->is_stopped(); }
    void pause() { command->pause(); }
    void resume() { command->resume(); }
    bool is_paused() const { return command->is_paused(); }
protected:
    std::shared_ptr<Command> command;
};

class Choice_Result : public Command_Result {
public:
    explicit Choice_Result(std::shared_ptr<Command> command) : Command_Result(command) {}
    int choice_index() const;
};

#endif
