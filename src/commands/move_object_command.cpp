#include "../../include/commands/move_object_command.hpp"
#include "../../include/map_object.hpp"
#include "../../include/utility/direction.hpp"


Move_Object_Command::Move_Object_Command(Map_Object& object, Direction dir,
    float pixels, bool skip_blocking, bool change_facing)
    : object(object), direction(dir), pixels(pixels),
    skip_blocking(skip_blocking), change_facing(change_facing),
    old_state(object.get_state()), complete(false) {
    if (direction == Direction::FORWARD)
        direction = object.get_direction();
    else if (direction == Direction::BACKWARD) {
        direction = opposite_direction(object.get_direction());
        this->change_facing = false;
    }
}

void Move_Object_Command::execute() {
    auto collision = object.move(direction, object.get_fps_independent_speed(),
        Collision_Check_Type::BOTH, change_facing);
    if (collision.passable())
        pixels -= object.get_fps_independent_speed();
    else if (skip_blocking)
        pixels = 0.0f;

    complete = stopped || object.is_stopped() || pixels <= 0.01f;
    if (complete) {
        object.set_state(old_state);
    }
}

bool Move_Object_Command::is_complete() const {
    return complete;
}