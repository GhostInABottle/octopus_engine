#ifndef HPP_MOVE_OBJECT_COMMAND
#define HPP_MOVE_OBJECT_COMMAND

#include "../commands/command.hpp"
#include "../direction.hpp"
#include <string>

class Game;
class Map_Object;

class Move_Object_Command : public Command {
public:
    struct Options {
        Map_Object& object;
        Direction direction;
        float pixels;
        bool skip_blocking;
        bool change_facing;
        bool animated;
        Options(Map_Object& obj, Direction dir, float pixels)
            : object(obj)
            , direction(dir)
            , pixels(pixels)
            , skip_blocking(true)
            , change_facing(true)
            , animated(true) {}
    };
    Move_Object_Command(Game& game, Options options);
    void execute() override;
    bool is_complete() const override;
private:
    Game& game;
    int object_id;
    Direction direction;
    float pixels;
    bool skip_blocking;
    bool change_facing;
    bool animated;
    std::string old_state;
    bool complete;
};

#endif