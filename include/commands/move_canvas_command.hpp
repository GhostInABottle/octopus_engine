#ifndef HPP_MOVE_CANVAS_COMMAND
#define HPP_MOVE_CANVAS_COMMAND

#include<optional>
#include "../xd/graphics/types.hpp"
#include "timed_command.hpp"

class Game;
class Base_Canvas;

class Move_Canvas_Command : public Timed_Command {
public:
    Move_Canvas_Command(Game& game, Base_Canvas& canvas, xd::vec2 pos, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Base_Canvas& canvas;
    xd::vec2 old_position;
    xd::vec2 new_position;
    bool complete;
};

#endif
