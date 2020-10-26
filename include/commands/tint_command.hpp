#ifndef HPP_TINT_SCREEN_COMMAND
#define HPP_TINT_SCREEN_COMMAND

#include "../xd/graphics/types.hpp"
#include "../command.hpp"

class Game;

enum class Tint_Target {
    MAP,
    SCREEN
};

class Tint_Command : public Command {
public:
    Tint_Command(Tint_Target target, Game& game, xd::vec4 color, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Tint_Target target;
    Game& game;
    xd::vec4 old_color;
    xd::vec4 new_color;
    long start_time;
    long duration;
    bool complete;
};

#endif
