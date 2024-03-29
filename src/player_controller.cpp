#include "../include/configurations.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/player_controller.hpp"
#include "../include/utility/direction.hpp"

namespace {
    static inline Direction check_edge_movement(const Map_Object& object, const Map_Object* other,
            const Map& map, Direction current_dir, float speed, int tolerance_pixels) {
        auto object_with_script = other && other->has_any_script();
        // Reduce the edge tolerance around scripted objects to make them easier to trigger
        auto factor = object_with_script ? (other->get_bounding_circle() ? 0.1f : 0.25f) : 1.0f;
        auto max_tolerance = static_cast<int>(std::ceil(factor * tolerance_pixels / speed));

        for (auto tolerance = max_tolerance; tolerance > 0; tolerance--) {
            auto vertical = direction_contains(current_dir, Direction::UP)
                || direction_contains(current_dir, Direction::DOWN);

            auto pos1{ object.get_position() };
            auto pos2{ pos1 };
            auto pos_change = tolerance * speed;
            if (vertical) {
                pos1.x -= pos_change;
                pos2.x += pos_change;
            } else {
                pos1.y -= pos_change;
                pos2.y += pos_change;
            }

            auto new_dir = Direction::NONE;
            if (map.passable(object, current_dir, pos1, speed).passable()) {
                new_dir = vertical ? Direction::LEFT : Direction::UP;
            } else if (map.passable(object, current_dir, pos2, speed).passable()) {
                new_dir = vertical ? Direction::RIGHT : Direction::DOWN;
            }

            if (new_dir != Direction::NONE) {
                return new_dir;
            }
        }

        return Direction::NONE;
    }
}

Player_Controller::Player_Controller(Game& game)
    : game(game),
    action_button(Configurations::get<std::string>("controls.action-button")),
    last_collision_check(game.ticks()),
    last_action_press(-1),
    last_action_direction(Direction::NONE),
    collision_check_delay(Configurations::get<int>("debug.collision-check-delay")),
    edge_tolerance_pixels(Configurations::get<int>("debug.edge-tolerance-pixels")) {}

void Player_Controller::update(Map_Object& object) {
    Direction direction = Direction::NONE;

    auto check_input = !object.is_disabled();
    if (check_input) {
        if (game.pressed("up")) direction = direction | Direction::UP;
        if (game.pressed("down")) direction = direction | Direction::DOWN;
        if (game.pressed("right")) direction = direction | Direction::RIGHT;
        if (game.pressed("left")) direction = direction | Direction::LEFT;
    }

    auto ticks = game.ticks();
    auto action_pressed = check_input && game.triggered(action_button);
    auto moved = direction != Direction::NONE;

    // Remember when the action key was last pressed
    if (action_pressed && moved) {
        last_action_press = ticks;
        last_action_direction = direction;
    } else if (moved && check_input && direction == last_action_direction) {
        // If the player pressed the action key while moving, remember it for a few frames
        action_pressed = last_action_press > 0
            && ticks - last_action_press <= collision_check_delay;
    }

    // Early exit if the player hasn't moved (or it's not time for regular check)
    auto time_to_check = ticks - last_collision_check > collision_check_delay;
    if (!moved && !action_pressed && !time_to_check) {
        if (check_input && object.get_state() == object.get_walk_state()) {
            // Reset to face state if not moving
            object.set_state(object.get_face_state());
        }

        return;
    }
    last_collision_check = ticks;

    auto speed = object.get_fps_independent_speed();
    auto map = game.get_map();
    auto check_type = Collision_Check_Type::BOTH;
    if (!moved) {
        direction = object.get_direction();
    }

    auto collision = map->passable(object, direction, check_type);

    // Check if stuck inside another object
    if (moved && collision.type == Collision_Type::OBJECT) {
        auto passable = false;
        for (int i = 1; i <= 8; i *= 2) {
            auto dir = static_cast<Direction>(i);
            // Only try directions other than the blocked one
            if ((dir & direction) != Direction::NONE) continue;

            auto other_collision = map->passable(object, dir, Collision_Check_Type::BOTH);
            if (other_collision.passable()) {
                passable = true;
                break;
            }
        }

        // If surrounded by objects in all directions, ignore object collisions
        if (!passable) {
            check_type = Collision_Check_Type::TILE;
            collision = map->passable(object, direction, check_type);
        }
    }

    // Try to move around edges
    auto change_facing = true;
    auto check_edges = moved && !is_diagonal(direction) && !collision.passable();
    if (check_edges) {
        auto other = collision.other_object;
        auto new_dir = check_edge_movement(object, other, *map, direction,
            speed, edge_tolerance_pixels);
        if (new_dir != Direction::NONE) {
            change_facing = object.get_direction() != direction;
            direction = direction | new_dir;
        }
    }

    // Actually move
    if (moved) {
        collision = object.move(direction, speed, check_type, change_facing);
        if (action_pressed && collision.passable()) {
            // Try again to see if there's an object in front of the player after moving
            collision = map->passable(object, direction, Collision_Check_Type::OBJECT);
        }
    }

    process_collision(object, collision, Collision_Type::OBJECT, moved, action_pressed);
    process_collision(object, collision, Collision_Type::AREA, moved, action_pressed);
}

void Player_Controller::process_collision(Map_Object& object, Collision_Record collision, Collision_Type type, bool moved, bool action_pressed) {
    Map_Object* old_object = nullptr;
    Map_Object* other = nullptr;
    bool new_collision = false;
    if (type == Collision_Type::OBJECT) {
        old_object = object.get_collision_object();
        other = collision.other_object;
        new_collision = other && old_object != other;
        if (new_collision) {
            object.set_collision_object(other);
        }
    } else {
        old_object = object.get_collision_area();
        other = collision.other_area;
        new_collision = other && old_object != other;
        // Check for movement because player has to manually collide with areas
        if (moved && new_collision) {
            object.set_collision_area(other);
        }
    }

    auto run_scripts = !object.is_disabled();
    auto touched = run_scripts && moved && new_collision && other->has_touch_script();
    auto triggered = run_scripts && action_pressed && other && other->has_trigger_script();

    if (touched || triggered) {
        object.set_triggered_object(other);

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
