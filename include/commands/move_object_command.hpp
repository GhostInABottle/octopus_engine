#ifndef HPP_MOVE_OBJECT_COMMAND
#define HPP_MOVE_OBJECT_COMMAND

#include "../commands/command.hpp"
#include "../direction.hpp"
#include <string>

class Game;
class Map_Object;

class Move_Object_Command : public Command {
public:
    Move_Object_Command(Game& game, Map_Object& object, Direction dir, float pixels,
        bool skip_blocking, bool change_facing);
    void execute() override;
    bool is_complete() const override;
private:
    Game& game;
    int object_id;
    Direction direction;
    float pixels;
    bool skip_blocking;
    bool change_facing;
    std::string old_state;
    bool complete;
};

#endif