#include "../../../include/commands/command_result.hpp"
#include "../../../include/commands/move_object_command.hpp"
#include "../../../include/commands/move_object_to_command.hpp"
#include "../../../include/commands/show_pose_command.hpp"
#include "../../../include/commands/update_color_command.hpp"
#include "../../../include/commands/update_opacity_command.hpp"
#include "../../../include/game.hpp"
#include "../../../include/log.hpp"
#include "../../../include/map/map.hpp"
#include "../../../include/map/map_object.hpp"
#include "../../../include/map/layers/object_layer.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/xd/graphics/types.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace detail {
    template <typename T>
    static T get_value(const sol::table& table, const std::string& key, Map_Object* map_obj) {
        sol::object object = table[key];

        if (!object.is<T>()) {
            LOGGER_E << "Invalid " << map_obj->get_name() << " move command option: " << key;
        }

        return object.as<T>();
    }

    static Move_Object_Command::Options move_options_from_table(Map_Object* object, const sol::table& table) {
        float distance = 0.0f;
        if (table["distance"].valid()) {
            distance = get_value<float>(table, "distance", object);
        } else {
            throw std::invalid_argument("Missing distance for move command");
        }

        auto dir = Direction::NONE;
        if (table["direction"].valid()) {
            dir = get_value<Direction>(table, "direction", object);
        } else {
            throw std::invalid_argument("Missing direction for move command");
        }

        Move_Object_Command::Options options{ *object, dir, distance };

        if (table["skip_blocking"].valid()) {
            options.skip_blocking = get_value<bool>(table, "skip_blocking", object);
        }

        if (table["change_facing"].valid()) {
            options.change_facing = get_value<bool>(table, "change_facing", object);
        }

        if (table["animated"].valid()) {
            options.animated = get_value<bool>(table, "animated", object);
        }

        return options;
    }
}

void bind_map_object_types(sol::state& lua, Game& game) {
    // Object draw order
    lua.new_enum<Map_Object::Draw_Order>("Draw_Order",
        {
            {"below", Map_Object::Draw_Order::BELOW},
            {"normal", Map_Object::Draw_Order::NORMAL},
            {"above", Map_Object::Draw_Order::ABOVE}
        }
    );
    // Script context
    lua.new_enum<Map_Object::Script_Context>("Script_Context",
        {
            {"global", Map_Object::Script_Context::GLOBAL},
            {"map", Map_Object::Script_Context::MAP}
        }
    );
    // Passthrough type
    lua.new_enum<Map_Object::Passthrough_Type>("Passthrough_Type",
        {
            {"initiator", Map_Object::Passthrough_Type::INITIATOR},
            {"receiver", Map_Object::Passthrough_Type::RECEIVER},
            {"both", Map_Object::Passthrough_Type::BOTH}
        }
    );
    // Outline conditions
    lua.new_enum<Map_Object::Outline_Condition>("Outline_Condition",
        {
            {"none", Map_Object::Outline_Condition::NONE},
            {"touched", Map_Object::Outline_Condition::TOUCHED},
            {"solid", Map_Object::Outline_Condition::SOLID},
            {"script", Map_Object::Outline_Condition::SCRIPT},
            {"proximate", Map_Object::Outline_Condition::PROXIMATE},
            {"never", Map_Object::Outline_Condition::NEVER},
        }
    );

    // Map object
    auto object_type = lua.new_usertype<Map_Object>("Map_Object");
    object_type["id"] = sol::property(&Map_Object::get_id);
    object_type["name"] = sol::property(&Map_Object::get_name, &Map_Object::set_name);
    object_type["type"] = sol::property(&Map_Object::get_type, &Map_Object::set_type);
    object_type["position"] = sol::property(&Map_Object::get_position, &Map_Object::set_position);
    object_type["size"] = sol::property(&Map_Object::get_size, &Map_Object::set_size);
    object_type["x"] = sol::property(&Map_Object::get_x, &Map_Object::set_x);
    object_type["y"] = sol::property(&Map_Object::get_y, &Map_Object::set_y);
    object_type["layer"] = sol::property(&Map_Object::get_layer,
        [&game](Map_Object* obj, Object_Layer* layer) {
            game.get_map()->move_object_to_layer(obj, layer);
        });
    object_type["color"] = sol::property(&Map_Object::get_color, &Map_Object::set_color);
    object_type["magnification"] = sol::property(&Map_Object::get_magnification, &Map_Object::set_magnification);
    object_type["opacity"] = sol::property(&Map_Object::get_opacity, &Map_Object::set_opacity);
    object_type["disabled"] = sol::property(&Map_Object::is_disabled, &Map_Object::set_disabled);
    object_type["stopped"] = sol::property(&Map_Object::is_stopped, &Map_Object::set_stopped);
    object_type["frozen"] = sol::property(&Map_Object::is_frozen, &Map_Object::set_frozen);
    object_type["passthrough"] = sol::property(&Map_Object::is_passthrough, &Map_Object::set_passthrough);
    object_type["passthrough_type"] = sol::property(&Map_Object::get_passthrough_type, &Map_Object::set_passthrough_type);
    object_type["pose"] = sol::property(&Map_Object::get_pose_name, &Map_Object::set_pose_name);
    object_type["sprite"] = sol::property(
        &Map_Object::get_sprite_filename,
        [&](Map_Object* obj, const std::string& filename) {
            obj->set_sprite(game, game.get_asset_manager(), filename);
        });
    object_type["last_marker"] = sol::property(&Map_Object::get_last_marker);
    object_type["passed_marker"] = &Map_Object::passed_marker;
    object_type["reset"] = &Map_Object::reset;
    object_type["set_sprite"] = [&](Map_Object* obj, const std::string& filename,
            std::optional<std::string> pose) {
        obj->set_sprite(game, game.get_asset_manager(), filename, pose.value_or(""));
    };
    object_type["show_pose"] = [&](Map_Object* obj, const std::string& pose_name,
            std::optional<std::string> state, std::optional<Direction> dir,
            std::optional<bool> reset_current_frame) {
        auto si = game.get_current_scripting_interface();
        auto holder_type = Show_Pose_Command::Holder_Type::MAP_OBJECT;
        Show_Pose_Command::Holder_Info holder_info{ holder_type, obj->get_id() };
        return si->register_command<Show_Pose_Command>(
            *game.get_map(), holder_info, pose_name, state.value_or(""),
            dir.value_or(Direction::NONE), reset_current_frame.value_or(true));
    };
    object_type["state"] = sol::property(&Map_Object::get_state, &Map_Object::set_state);
    object_type["walk_state"] = sol::property(&Map_Object::get_walk_state, &Map_Object::set_walk_state);
    object_type["face_state"] = sol::property(&Map_Object::get_face_state, &Map_Object::set_face_state);
    object_type["visible"] = sol::property(&Map_Object::is_visible, &Map_Object::set_visible);
    object_type["script_context"] = sol::property(&Map_Object::get_script_context, &Map_Object::set_script_context);
    object_type["script"] = sol::property(&Map_Object::get_trigger_script, &Map_Object::set_trigger_script);
    object_type["trigger_script"] = sol::property(&Map_Object::get_trigger_script, &Map_Object::set_trigger_script);
    object_type["touch_script"] = sol::property(&Map_Object::get_touch_script, &Map_Object::set_touch_script);
    object_type["leave_script"] = sol::property(&Map_Object::get_leave_script, &Map_Object::set_leave_script);
    object_type["overrides_tile_collision"] = sol::property(&Map_Object::overrides_tile_collision,
        &Map_Object::set_override_tile_collision);
    object_type["strict_multidirectional_movement"] = sol::property(&Map_Object::get_strict_multidirectional_movement,
        &Map_Object::set_strict_multidirectional_movement);
    object_type["uses_layer_color"] = sol::property(&Map_Object::uses_layer_color, &Map_Object::set_use_layer_color);
    object_type["triggered_object"] = sol::property(&Map_Object::get_triggered_object, &Map_Object::set_triggered_object);
    object_type["collision_object"] = sol::property(&Map_Object::get_collision_object, &Map_Object::set_collision_object);
    object_type["collision_area"] = sol::property(&Map_Object::get_collision_area, &Map_Object::set_collision_area);
    object_type["proximate_object"] = sol::property(&Map_Object::get_proximate_object, &Map_Object::set_proximate_object);
    object_type["proximity_distance"] = sol::property(&Map_Object::proximity_distance, &Map_Object::set_proximity_distance);
    object_type["outlined"] = sol::property(&Map_Object::is_outlined, &Map_Object::set_outlined);
    object_type["outline_conditions"] = sol::property(&Map_Object::get_outline_conditions,
        &Map_Object::set_outline_conditions);
    object_type["outline_color"] = sol::property(&Map_Object::get_outline_color, &Map_Object::set_outline_color);
    object_type["outlined_object"] = sol::property(
        [&](Map_Object* object) {
            auto id = object->get_outlined_object_id();
            return id > -1 ? game.get_map()->get_object(id) : nullptr;
        },
        [&](Map_Object* object, Map_Object* other) {
            auto map = game.get_map();
            if (!other) {
                other = map->get_object(object->get_outlined_object_id());
                if (other) other->set_outlining_object(nullptr);
                object->set_outlined_object_id(-1);
                return;
            }
            object->set_outlined_object_id(other->get_id());
            other->set_outlining_object(object);
        }
    );
    object_type["outlining_object"] = sol::property(&Map_Object::get_outlining_object);
    object_type["draw_order"] = sol::property(&Map_Object::get_draw_order, &Map_Object::set_draw_order);
    object_type["real_position"] = sol::property(&Map_Object::get_real_position);
    object_type["positioned_bounding_box"] = sol::property(&Map_Object::get_positioned_bounding_box);
    object_type["positioned_bounding_circle"] = sol::property(&Map_Object::get_positioned_bounding_circle);
    object_type["centered_position"] = sol::property(&Map_Object::get_centered_position);
    object_type["bounding_box"] = sol::property(&Map_Object::get_bounding_box, &Map_Object::set_bounding_box);
    object_type["bounding_circle"] = sol::property(&Map_Object::get_bounding_circle, &Map_Object::set_bounding_circle);
    object_type["speed"] = sol::property(&Map_Object::get_speed, &Map_Object::set_speed);
    object_type["movement_speed"] = sol::property(&Map_Object::get_movement_speed, &Map_Object::set_movement_speed);
    object_type["animation_speed"] = sol::property(&Map_Object::get_animation_speed, &Map_Object::set_animation_speed);
    object_type["fps_independent_speed"] = sol::property(&Map_Object::get_fps_independent_speed);
    object_type["angle"] = sol::property(&Map_Object::get_angle, &Map_Object::set_angle);
    object_type["direction"] = sol::property(
        [](Map_Object* obj) {
            return static_cast<int>(obj->get_direction());
        },
        [](Map_Object* obj, int dir) {
            obj->set_direction(static_cast<Direction>(dir));
        }
    );
    object_type["sprite_magnification"] = sol::property(&Map_Object::get_sprite_magnification);
    object_type["sfx_attenuation"] = sol::property(&Map_Object::is_sound_attenuation_enabled,
        &Map_Object::set_sound_attenuation_enabled);
    object_type["get_property"] = &Map_Object::get_property;
    object_type["set_property"] = &Map_Object::set_property;
    object_type["move_to"] = sol::overload(
        [&](Map_Object* obj, float x, float y) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                *game.get_map(), *obj, x, y);
        },
        [&](Map_Object* obj, xd::vec2 pos) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                *game.get_map(), *obj, pos.x, pos.y);
        },
            [&](Map_Object* obj, float x, float y, bool keep_trying) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                *game.get_map(), *obj, x, y,
                Collision_Check_Type::BOTH, keep_trying);
        },
            [&](Map_Object* obj, xd::vec2 pos, bool keep_trying) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                *game.get_map(), *obj, pos.x, pos.y,
                Collision_Check_Type::BOTH, keep_trying);
        }
    );
    object_type["move"] = sol::overload(
        [&](Map_Object* obj, int dir, float pixels) {
            auto si = game.get_current_scripting_interface();
            Move_Object_Command::Options options{ *obj, static_cast<Direction>(dir), pixels };
            return si->register_command<Move_Object_Command>(game, options);
        },
        [&](Map_Object* obj, const sol::table& table) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Object_Command>(game,
                detail::move_options_from_table(obj, table));
        }
    );
    object_type["update_opacity"] = [&](Map_Object* obj, float opacity, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
            game, *obj, opacity, duration);
    };
    object_type["update_color"] = [&](Map_Object* obj, xd::vec4 color, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Color_Command>(
            game, *obj, color, duration);
    };
    object_type["face"] = sol::overload(
        (void (Map_Object::*)(xd::vec2)) &Map_Object::face,
        (void (Map_Object::*)(float, float)) &Map_Object::face,
        (void (Map_Object::*)(const Map_Object&)) &Map_Object::face,
        [&](Map_Object* obj, int dir) {
            obj->face(static_cast<Direction>(dir));
        }
    );
    object_type["run_script"] = &Map_Object::run_trigger_script;
    object_type["run_trigger_script"] = &Map_Object::run_trigger_script;
    object_type["run_touch_script"] = &Map_Object::run_touch_script;
    object_type["run_leave_script"] = &Map_Object::run_leave_script;
    object_type["add_linked_object"] = &Map_Object::add_linked_object;
    object_type["remove_linked_object"] = &Map_Object::remove_linked_object;
    object_type[sol::meta_function::to_string] = [](const Map_Object& val) {
        std::stringstream ss;
        ss << "Map_Object(id: " << val.get_id() << ", name: " << val.get_name() << ")";
        return ss.str();
    };
}
