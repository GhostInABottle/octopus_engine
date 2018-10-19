#ifndef HPP_MOVE_OBJECT_COMMAND
#define HPP_MOVE_OBJECT_COMMAND

#include <string>
#include "../direction.hpp"
#include "../command.hpp"

class Map_Object;

class Move_Object_Command : public Command {
public:
    Move_Object_Command(Map_Object& object, Direction dir, float pixels,
        bool skip_blocking, bool change_facing);
    void execute();
    bool is_complete() const;
private:
    Map_Object& object;
    Direction direction;
    float pixels;
    bool skip_blocking;
    bool change_facing;
    std::string old_state;
    bool complete;
};

#endif