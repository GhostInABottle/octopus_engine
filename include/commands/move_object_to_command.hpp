#ifndef HPP_MOVE_OBJECT_TO_COMMAND
#define HPP_MOVE_OBJECT_TO_COMMAND

#include <memory>
#include <deque>
#include <xd/graphics/types.hpp>
#include "../command.hpp"
#include "../direction.hpp"
#include "../collision_check_types.hpp"

struct Node;
class Map_Object;
class Map;

class Move_Object_To_Command : public Command {
public:
    Move_Object_To_Command(Map& map, Map_Object& object, float x, float y,
        Collision_Check_Types check_type = Collision_Check_Types::BOTH,
        bool keep_trying = false);
    ~Move_Object_To_Command();
    void execute();
    bool is_complete() const {
        return is_complete(0);
    }
    bool is_complete(int ticks) const;
private:
    void init();
    Map& map;
    Map_Object& object;
    xd::vec2 destination;
    std::deque<Direction> path;
    float pixels;
    bool keep_trying;
    int last_attempt_time;
    bool blocked;
    Collision_Check_Types check_type;
    std::unique_ptr<Node> nearest;
};

#endif