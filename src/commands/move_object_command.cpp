#include "../../include/commands/move_object_command.hpp"
#include "../../include/game.hpp"
#include "../../include/map/map.hpp"
#include "../../include/map/map_object.hpp"
#include "../../include/utility/direction.hpp"

Move_Object_Command::Move_Object_Command(Game& game, Move_Object_Command::Options options)
        : game(game)
        , object_id(options.object.get_id())
        , direction(options.direction)
        , pixels(options.pixels)
        , skip_blocking(options.skip_blocking)
        , change_facing(options.change_facing)
        , animated(options.animated)
        , old_state(options.object.get_state())
        , complete(false) {
    if (direction == Direction::FORWARD) {
        direction = options.object.get_direction();
    } else if (direction == Direction::BACKWARD) {
        direction = opposite_direction(options.object.get_direction());
        change_facing = false;
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
            Collision_Check_Type::BOTH, change_facing, animated);
        if (collision.passable()) {
            pixels -= object->get_fps_independent_speed();
        } else if (skip_blocking) {
            pixels = 0.0f;
        }
    }

    complete = stopped || object->is_stopped() || pixels <= 0.01f;
    if (complete || paused) {
        object->set_state(old_state);
    }
}

bool Move_Object_Command::is_complete() const {
    return complete || force_stopped;
}
