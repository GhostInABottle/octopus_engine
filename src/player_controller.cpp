#include "../include/configurations.hpp"
#include "../include/game.hpp"
#include "../include/map/map.hpp"
#include "../include/map/map_object.hpp"
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
            auto check_type = Collision_Check_Type::BOTH;
            if (map.passable({ object, current_dir, check_type, pos1, speed }).passable()) {
                new_dir = vertical ? Direction::LEFT : Direction::UP;
            } else if (map.passable({ object, current_dir, check_type, pos2, speed }).passable()) {
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
    collision_check_delay(Configurations::get<int>("player.collision-check-delay")),
    edge_tolerance_pixels(Configurations::get<int>("player.edge-tolerance-pixels")) {}

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
    if (!moved) {
        direction = object.get_direction();
    }

    auto collision = map->passable({ object, direction });


    auto check_type = Collision_Check_Type::BOTH;
    // Check if stuck inside another object
    if (moved && collision.type == Collision_Type::OBJECT) {
        auto passable = false;
        // All valid directions starting with non-diagonal ones
        const static int dirs[] = { 1, 2, 4, 8, 3, 6, 9, 12 };
        for (int i = 0; i < 8; ++i) {
            auto dir = static_cast<Direction>(dirs[i]);
            // Only try directions other than the blocked one
            if (dir == direction) continue;

            auto other_collision = map->passable({ object, dir });
            if (other_collision.passable()) {
                passable = true;
                break;
            }
        }

        // If surrounded by objects in all directions, ignore object collisions
        if (!passable) {
            check_type = Collision_Check_Type::TILE;
            collision = map->passable({ object, direction, check_type });
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
            collision = map->passable({ object, direction, Collision_Check_Type::OBJECT });
        }
    }

    if (!process_collision(object, collision, Collision_Type::OBJECT, moved, action_pressed)) {
        // Object interactions have priority over area ones
        process_collision(object, collision, Collision_Type::AREA, moved, action_pressed);
    }
}

bool Player_Controller::process_collision(Map_Object& object, Collision_Record collision, Collision_Type type, bool moved, bool action_pressed) {
    Map_Object* old_object = nullptr;
    Map_Object* collision_object = nullptr;
    bool new_collision = false;
    if (type == Collision_Type::OBJECT) {
        old_object = object.get_collision_object();
        collision_object = collision.other_object;
        new_collision = collision_object && old_object != collision_object;
        if (new_collision) {
            object.set_collision_object(collision_object);
        }
    } else {
        old_object = object.get_collision_area();
        collision_object = collision.other_area;
        new_collision = collision_object && old_object != collision_object;
        if (moved && new_collision) {
            // Check for movement because player has to manually collide with areas
            object.set_collision_area(collision_object);
        } else if (new_collision && !old_object) {
            // Prevent triggering an area when it's not the active one
            // e.g. pressing action button on the edge of the area without entering it
            collision_object = nullptr;
        }
    }

    auto other_object = collision_object;
    auto run_scripts = !object.is_disabled();
    auto touched = run_scripts && moved && new_collision
        && collision_object->has_touch_script();
    auto triggered = run_scripts && action_pressed
        && collision_object && collision_object->has_trigger_script();

    object.set_proximate_object(collision.proximate_object);
    auto check_proximate_object = run_scripts && !collision_object
        && action_pressed && collision.proximate_object;
    if (check_proximate_object) {
        other_object = collision.proximate_object;
        triggered = collision.proximate_object->has_trigger_script();
    }

    if (touched || triggered) {
        object.set_triggered_object(other_object);

        if (touched) {
            other_object->run_touch_script();
        }
        if (triggered) {
            other_object->run_trigger_script();
        }

        return true;
    }

    if (old_object && !other_object) {
        if (run_scripts && moved && old_object->has_leave_script()) {
            old_object->run_leave_script();
        }

        if (type == Collision_Type::OBJECT) {
            object.set_collision_object(nullptr);
        } else {
            object.set_collision_area(nullptr);
        }
    }

    return false;
}
