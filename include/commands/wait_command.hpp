#ifndef HPP_WAIT_COMMAND
#define HPP_WAIT_COMMAND

#include "timed_command.hpp"

class Game;

class Wait_Command : public Timed_Command {
public:
    Wait_Command(Game& game, int duration, int start = -1);
    void execute() override {}
    bool is_complete() const override;
    bool is_complete(int ticks) const override;
};

#endif
