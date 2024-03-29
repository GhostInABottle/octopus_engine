#ifndef HPP_TINT_COMMAND
#define HPP_TINT_COMMAND

#include "../xd/glm.hpp"
#include "timed_command.hpp"

class Game;

enum class Tint_Target {
    MAP,
    SCREEN
};

// Update the map or screen tint color
class Tint_Command : public Timed_Command {
public:
    Tint_Command(Tint_Target target, Game& game, xd::vec4 color, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Tint_Target target;
    xd::vec4 old_color;
    xd::vec4 new_color;
    bool complete;
};

#endif
