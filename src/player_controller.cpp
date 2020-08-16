#include "../include/player_controller.hpp"
#include "../include/object_layer.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/game.hpp"
#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility/direction.hpp"
#include "../include/configurations.hpp"

Player_Controller::Player_Controller(Game& game) : game(game) {}

void Player_Controller::update(Map_Object& object) {
    if (object.is_disabled())
        return;

    Direction direction = Direction::NONE;
    if (game.pressed("up"))
        direction = direction | Direction::UP;
    if (game.pressed("down"))
        direction = direction | Direction::DOWN;
    if (game.pressed("right"))
        direction = direction | Direction::RIGHT;
    if (game.pressed("left"))
        direction = direction | Direction::LEFT;

    static std::string action_button =
        Configurations::get<std::string>("controls.action-button");
    bool action_pressed = game.triggered(action_button);
    bool moved = direction != Direction::NONE;
    if (!moved && !action_pressed) {
        if (object.get_state() == object.get_walk_state())
            object.set_state(object.get_face_state());
        return;
    }
    auto collision = object.move(direction, object.get_speed());
    // Check if stuck inside another object
    if (moved && collision.type == Collision_Type::OBJECT) {
        bool passable = false;
        for (int i = 1; i <= 8; i *= 2) {
            auto dir = static_cast<Direction>(i);
            if (dir == direction) continue;
            auto rec = game.get_map()->passable(object, dir);
            auto& others = rec.other_objects;
            if (others.find(collision.other_object->get_name()) == others.end()) {
                passable = true;
                break;
            }
        }
        // If surrounded by object in all directions, ignore object collisions
        if (!passable)
            collision = object.move(direction, object.get_speed(),
                Collision_Check_Type::TILE);
    }

    process_collision(object, collision, Collision_Type::OBJECT, action_pressed);
    process_collision(object, collision, Collision_Type::AREA, action_pressed);

    // Check collision one more time to outline any touched objects
    if (object.get_collision_object()) return;
    auto touching = game.get_map()->passable(object, object.get_direction(), Collision_Check_Type::OBJECT);
    if (touching.type == Collision_Type::OBJECT) {
        process_collision(object, touching, Collision_Type::OBJECT, false);
    }
}

void Player_Controller::process_collision(Map_Object& object, Collision_Record collision, Collision_Type type, bool action_pressed) {
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
    auto touched = other && other->has_touch_script() && other != old_object;
    auto triggered = other && action_pressed && other->has_trigger_script();
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
        if (old_object && old_object->has_leave_script()) {
            old_object->run_leave_script();
        }
        if (type == Collision_Type::OBJECT) {
            object.set_collision_object(nullptr);
        } else {
            object.set_collision_area(nullptr);
        }
    }
}
