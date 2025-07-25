#include "../script_bindings.hpp"
#include "../scripting_interface.hpp"
#include "../../camera.hpp"
#include "../../commands/command_result.hpp"
#include "../../commands/move_camera_command.hpp"
#include "../../commands/shake_screen_command.hpp"
#include "../../commands/tint_command.hpp"
#include "../../commands/zoom_command.hpp"
#include "../../game.hpp"
#include "../../map/map_object.hpp"
#include "../../utility/color.hpp"
#include "../../xd/vendor/sol/sol.hpp"
#include <string>

void bind_camera_types(sol::state& lua, Game& game) {
    // Camera tracking and effects
    auto camera_type = lua.new_usertype<Camera>("Camera");
    camera_type["screen_tint"] = sol::property(&Camera::get_screen_tint, &Camera::set_screen_tint);
    camera_type["map_tint"] = sol::property(&Camera::get_map_tint, &Camera::set_map_tint);
    camera_type["position"] = sol::property(&Camera::get_position, &Camera::set_position);
    camera_type["position_bounds"] = sol::readonly_property(&Camera::get_position_bounds);
    camera_type["tracked_object"] = sol::property(&Camera::get_object, &Camera::set_object);
    camera_type["object_center_offset"] = sol::property(&Camera::get_object_center_offset,
        &Camera::set_object_center_offset);
    camera_type["is_shaking"] = sol::property(&Camera::is_shaking);
    camera_type["get_centered_position"] = sol::overload(
        sol::resolve<xd::vec2(xd::vec2) const>(&Camera::get_centered_position),
        sol::resolve<xd::vec2(const Map_Object&) const>(&Camera::get_centered_position));
    camera_type["set_shader"] = &Camera::set_shader;
    camera_type["move"] = [&](Camera& camera, int dir, float pixels, float speed) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Move_Camera_Command>(camera, static_cast<Direction>(dir), pixels, speed);
    };
    camera_type["move_to"] = sol::overload(
        [&](Camera& camera, float x, float y, float speed) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Camera_Command>(camera, x, y, speed);
        },
        [&](Camera& camera, xd::vec2 pos, float speed) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Camera_Command>(camera, pos.x, pos.y, speed);
        },
        [&](Camera& camera, Map_Object& object, float speed) {
            auto pos = camera.get_centered_position(object);
            auto si = game.get_current_scripting_interface();
            return si->register_command<Move_Camera_Command>(camera, pos.x, pos.y, speed);
        }
    );
    camera_type["tint_screen"] = sol::overload(
        [&](Camera&, xd::vec4 color, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Tint_Command>(Tint_Target::SCREEN, game, color, duration);
        },
        [&](Camera&, const std::string& hex_color, long duration) {
            auto si = game.get_current_scripting_interface();
            auto color = hex_to_color(hex_color);
            return si->register_command<Tint_Command>(Tint_Target::SCREEN, game, color, duration);
        }
    );
    camera_type["tint_map"] = sol::overload(
        [&](Camera&, xd::vec4 color, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Tint_Command>(Tint_Target::MAP, game, color, duration);
        },
        [&](Camera&, const std::string& hex_color, long duration) {
            auto si = game.get_current_scripting_interface();
            auto color = hex_to_color(hex_color);
            return si->register_command<Tint_Command>(Tint_Target::MAP, game, color, duration);
        }
    );
    camera_type["zoom"] = [&](Camera&, float scale, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Zoom_Command>(game, scale, duration);
    };
    camera_type["center_at"] = sol::overload(
        (void (Camera::*)(xd::vec2)) & Camera::center_at,
        (void (Camera::*)(const Map_Object&)) & Camera::center_at,
        [](Camera& camera, float x, float y) { camera.center_at(xd::vec2(x, y)); }
    );
    camera_type["track_object"] = [&](Camera& camera, Map_Object& object) { camera.set_object(&object); };
    camera_type["stop_tracking"] = [&](Camera& camera) { camera.set_object(nullptr); };
    camera_type["start_shaking"] = sol::overload(
        [](Camera& camera, xd::vec2 strength, xd::vec2 speed) {
            camera.start_shaking(strength, speed);
        },
        [](Camera& camera, float strength, float speed) {
            camera.start_shaking(xd::vec2{strength, 0}, xd::vec2{speed, 0});
        }
    );
    camera_type["cease_shaking"] = &Camera::cease_shaking;
    camera_type["shake_screen"] = sol::overload(
        [&](Camera*, xd::vec2 strength, xd::vec2 speed, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Shake_Screen_Command>(game,
                strength, speed, duration);
        },
        [&](Camera*, float strength, float speed, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Shake_Screen_Command>(game,
                xd::vec2{strength, 0}, xd::vec2{speed, 0}, duration);
        }
    );
}
