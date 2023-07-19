#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/game.hpp"
#include "../../../include/camera.hpp"
#include "../../../include/map_object.hpp"
#include "../../../include/commands/command_result.hpp"
#include "../../../include/commands/tint_command.hpp"
#include "../../../include/commands/zoom_command.hpp"
#include "../../../include/commands/shake_screen_command.hpp"
#include "../../../include/commands/move_camera_command.hpp"
#include "../../../include/utility/color.hpp"
#include "../../../include/xd/lua.hpp"
#include <string>

void bind_camera_types(sol::state& lua, Game& game) {
    // Camera tracking and effects
    auto camera_type = lua.new_usertype<Camera>("Camera");
    camera_type["screen_tint"] = sol::property(&Camera::get_screen_tint, &Camera::set_screen_tint);
    camera_type["map_tint"] = sol::property(&Camera::get_map_tint, &Camera::set_map_tint);
    camera_type["position"] = sol::property(&Camera::get_position, &Camera::set_position);
    camera_type["position_bounds"] = sol::readonly_property(&Camera::get_position_bounds);
    camera_type["tracked_object"] = sol::property(&Camera::get_object, &Camera::set_object);
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
        [&](Camera& camera, xd::vec4 color, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Tint_Command>(Tint_Target::SCREEN, game, color, duration);
        },
        [&](Camera& camera, const std::string& hex_color, long duration) {
            auto si = game.get_current_scripting_interface();
            auto color = hex_to_color(hex_color);
            return si->register_command<Tint_Command>(Tint_Target::SCREEN, game, color, duration);
        }
    );
    camera_type["tint_map"] = sol::overload(
        [&](Camera& camera, xd::vec4 color, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Tint_Command>(Tint_Target::MAP, game, color, duration);
        },
        [&](Camera& camera, const std::string& hex_color, long duration) {
            auto si = game.get_current_scripting_interface();
            auto color = hex_to_color(hex_color);
            return si->register_command<Tint_Command>(Tint_Target::MAP, game, color, duration);
        }
    );
    camera_type["zoom"] = [&](Camera& camera, float scale, long duration) {
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
    camera_type["start_shaking"] = &Camera::start_shaking;
    camera_type["cease_shaking"] = &Camera::cease_shaking;
    camera_type["shake_screen"] = [&](Camera* camera, float strength, float speed, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Shake_Screen_Command>(game, strength, speed, duration);
    };
}
