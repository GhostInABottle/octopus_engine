#include "../../include/commands/move_object_command.hpp"
#include "../../include/map_object.hpp"
#include "../../include/direction_utilities.hpp"


Move_Object_Command::Move_Object_Command(Map_Object& object, Direction dir,
    float pixels, bool skip_blocking, bool change_facing)
    : object(object), direction(dir), pixels(pixels),
    skip_blocking(skip_blocking), change_facing(change_facing) {
    if (direction == Direction::FORWARD)
        direction = object.get_direction();
    else if (direction == Direction::BACKWARD) {
        direction = opposite_direction(object.get_direction());
        this->change_facing = false;
    }
}

void Move_Object_Command::execute() {
    auto collision = object.move(direction, object.get_speed(),
        Collision_Check_Types::BOTH, change_facing);
    if (collision.passable())
        pixels -= object.get_speed();
    else if (skip_blocking)
        pixels = 0.0f;
}

bool Move_Object_Command::is_complete() const {
    bool complete = stopped || object.is_stopped() || pixels <= 0;
    if (complete)
        object.update_state("FACE");
    return complete;
}