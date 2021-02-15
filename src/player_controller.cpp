#include "../include/player_controller.hpp"
#include "../include/object_layer.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/game.hpp"
#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility/direction.hpp"
#include "../include/configurations.hpp"

Player_Controller::Player_Controller(Game& game)
    : game(game),
    action_button(Configurations::get<std::string>("controls.action-button")),
    last_collision_check(game.ticks()),
    collision_check_delay(Configurations::get<int>("debug.collision-check-delay")),
    edge_tolerance_pixels(Configurations::get<int>("debug.edge-tolerance-pixels")) {}

void Player_Controller::update(Map_Object& object) {
    Direction direction = Direction::NONE;

    auto check_input = !object.is_disabled();
    if (check_input) {
        if (game.pressed("up"))
            direction = direction | Direction::UP;
        if (game.pressed("down"))
            direction = direction | Direction::DOWN;
        if (game.pressed("right"))
            direction = direction | Direction::RIGHT;
        if (game.pressed("left"))
            direction = direction | Direction::LEFT;
    }

    auto action_pressed = check_input && game.triggered(action_button);
    auto moved = direction != Direction::NONE;
    auto time_to_check = game.ticks() - last_collision_check > collision_check_delay;
    if (!moved && !action_pressed && !time_to_check) {
        if (check_input && object.get_state() == object.get_walk_state())
            object.set_state(object.get_face_state());
        return;
    }
    last_collision_check = game.ticks();

    auto speed = object.get_fps_independent_speed();
    auto map = game.get_map();
    auto check_type = Collision_Check_Type::BOTH;
    if (direction == Direction::NONE) {
        direction = object.get_direction();
    }

    auto collision = map->passable(object, direction, check_type);

    // Check if stuck inside another object
    if (moved && collision.type == Collision_Type::OBJECT) {
        auto passable = false;
        for (int i = 1; i <= 8; i *= 2) {
            auto dir = static_cast<Direction>(i);
            if (dir == direction) continue;
            auto rec = map->passable(object, dir);
            auto& others = rec.other_objects;
            if (others.find(collision.other_object->get_name()) == others.end()) {
                passable = true;
                break;
            }
        }
        // If surrounded by object in all directions, ignore object collisions
        if (!passable) {
            check_type = Collision_Check_Type::TILE;
            collision = map->passable(object, direction, check_type);
        }
    }

    // Try to move around edges
    auto change_facing = true;
    auto check_edges = moved && !is_diagonal(direction) && !collision.passable()
        && (!collision.other_object || !collision.other_object->has_any_script());
    if (check_edges) {
        auto vertical = direction == Direction::UP || direction == Direction::DOWN;
        auto horizontal = direction == Direction::LEFT || direction == Direction::RIGHT;
        auto pos1{object.get_position()};
        auto pos2{pos1};
        if (vertical) {
            pos1.x -= edge_tolerance_pixels;
            pos2.x += edge_tolerance_pixels;
        } else {
            pos1.y -= edge_tolerance_pixels;
            pos2.y += edge_tolerance_pixels;
        }

        auto new_dir = Direction::NONE;
        if (map->passable(object, direction, pos1, speed).passable()) {
            new_dir = vertical ? Direction::LEFT : Direction::UP;
        } else if (map->passable(object, direction, pos2, speed).passable()) {
            new_dir = vertical ? Direction::RIGHT : Direction::DOWN;
        }

        if (new_dir != Direction::NONE) {
            change_facing = object.get_direction() != direction;
            direction = new_dir;
        }
    }

    if (moved) {
        collision = object.move(direction, speed, check_type, change_facing);
    }

    process_collision(object, collision, Collision_Type::OBJECT, moved, action_pressed);
    process_collision(object, collision, Collision_Type::AREA, moved, action_pressed);
}

void Player_Controller::process_collision(Map_Object& object, Collision_Record collision, Collision_Type type, bool moved, bool action_pressed) {
    Map_Object* old_object = nullptr;
    Map_Object* other = nullptr;
    if (type == Collision_Type::OBJECT) {
        old_object = object.get_collision_object();
        other = collision.other_object;
        if (other)
            object.set_collision_object(other);
    } else {
        old_object = object.get_collision_area();
        other = collision.other_area;
        if (other)
            object.set_collision_area(other);
    }

    auto run_scripts = !object.is_disabled();
    auto touched = run_scripts && moved && other && other->has_touch_script() && other != old_object;
    auto triggered = run_scripts && action_pressed && other && other->has_trigger_script();
    if (touched || triggered) {
        object.set_triggered_object(other);
        if (other->is_player_facing()) {
            other->face(object);
        }
        if (touched) {
            other->run_touch_script();
        }
        if (triggered) {
            other->run_trigger_script();
        }
    } else if (!other) {
        if (run_scripts && moved && old_object && old_object->has_leave_script()) {
            old_object->run_leave_script();
        }
        if (type == Collision_Type::OBJECT) {
            object.set_collision_object(nullptr);
        } else {
            object.set_collision_area(nullptr);
        }
    }
}
