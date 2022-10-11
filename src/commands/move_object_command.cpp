#include "../../include/commands/move_object_command.hpp"
#include "../../include/map_object.hpp"
#include "../../include/game.hpp"
#include "../../include/map.hpp"
#include "../../include/utility/direction.hpp"


Move_Object_Command::Move_Object_Command(Game& game, Map_Object& object, Direction dir,
        float pixels, bool skip_blocking, bool change_facing)
        : game(game)
        , object_id(object.get_id())
        , direction(dir)
        , pixels(pixels),
    skip_blocking(skip_blocking), change_facing(change_facing),
    old_state(object.get_state()), complete(false) {
    if (direction == Direction::FORWARD)
        direction = object.get_direction();
    else if (direction == Direction::BACKWARD) {
        direction = opposite_direction(object.get_direction());
        this->change_facing = false;
    }
    map_ptr = game.get_map();
}

void Move_Object_Command::execute() {
    auto map = game.get_map();
    auto object = map->get_object(object_id);
    if (!object) {
        complete = true;
        return;
    }
    if (!paused) {
        auto collision = object->move(direction, object->get_fps_independent_speed(),
            Collision_Check_Type::BOTH, change_facing);
        if (collision.passable())
            pixels -= object->get_fps_independent_speed();
        else if (skip_blocking)
            pixels = 0.0f;
    }

    complete = stopped || object->is_stopped() || pixels <= 0.01f;
    if (complete || paused) {
        object->set_state(old_state);
    }
}

bool Move_Object_Command::is_complete() const {
    return complete;
}