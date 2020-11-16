#ifndef HPP_SHAKE_SCREEN_COMMAND
#define HPP_SHAKE_SCREEN_COMMAND

#include "timed_command.hpp"

class Game;

class Shake_Screen_Command : public Timed_Command {
public:
    Shake_Screen_Command(Game& game, float strength, float speed, long duration);
    void execute()  override {}
    bool is_complete() const override;
};

#endif
