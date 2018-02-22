#include "../include/player_controller.hpp"
#include "../include/object_layer.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/game.hpp"
#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility.hpp"
#include "../include/direction_utilities.hpp"
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
        if (object.get_state() == "WALK")
            object.update_state("FACE");
        return;
    }
    auto collision = object.move(direction, object.get_speed());
    // Check if stuck inside another object
    if (moved && !collision.passable() &&
            collision.type == Collision_Types::OBJECT) {
        bool passable = false;
        auto check_type = Collision_Check_Types::MULTI_OBJECTS;
        auto map = game.get_map();
        for (int i = 1; i <= 8; i *= 2) {
            auto dir = static_cast<Direction>(i);
            auto rec = map->passable(object, dir, check_type);
            auto& others = rec.other_objects;
            if (others.find(collision.other_object->get_name()) == others.end()) {
                passable = true;
                break;
            }
        }
        // If surrounded by object in all directions, ignore object collisions
        if (!passable)
            collision = object.move(direction, object.get_speed(),
                Collision_Check_Types::TILE);
    }

    // If action button pressed, activate NPC
    if (action_pressed && collision.other_object &&
            collision.input_triggerable()) {
        object.set_triggered_object(collision.other_object);
        collision.other_object->face(object);
        collision.other_object->run_trigger_script();
    }
    // Check if inside a collision area
    if (collision.is_area()) {
        auto area = collision.other_object;
        if (area != object.get_collision_area()) {
            object.set_collision_area(area);
            // Automatically trigger regular areas
            if (area && moved && !collision.input_triggerable()) {
                object.set_triggered_object(area);
                area->run_trigger_script();
            }
        }
    } else if (collision.type == Collision_Types::NONE) {
        auto old_area = object.get_collision_area();
        if (old_area)
            old_area->run_exit_script();
        object.set_collision_area(nullptr);
    }

}
