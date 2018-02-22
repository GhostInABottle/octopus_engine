#ifndef HPP_WAIT_COMMAND
#define HPP_WAIT_COMMAND

#include "../command.hpp"

class Game;

class Wait_Command : public Command {
public:
    Wait_Command(Game& game, int duration, int start = -1);
    void execute() {}
    bool is_complete() const;
    bool is_complete(int ticks) const;
private:
    Game& game;
    int start_time;
    int duration;
};



#endif
