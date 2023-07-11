#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/game.hpp"
#include "../../../include/map.hpp"
#include "../../../include/map_object.hpp"
#include "../../../include/log.hpp"
#include "../../../include/command_result.hpp"
#include "../../../include/commands/show_text_command.hpp"
#include "../../../include/commands/show_pose_command.hpp"
#include "../../../include/commands/wait_command.hpp"
#include "../../../include/commands/move_object_to_command.hpp"
#include "../../../include/utility/direction.hpp"
#include "../../../include/xd/lua.hpp"
#include <string>
#include <optional>
#include <memory>

void bind_utility_types(sol::state& lua, Game& game) {
    // Waiting for duration / function result
    auto wait = [](Game& game, int duration) {
        bool was_paused = game.is_paused();
        int old_time = was_paused ? game.window_ticks() : game.ticks();
        auto& scheduler = game.get_current_scripting_interface()->get_scheduler();
        scheduler.yield([&game, was_paused, old_time, duration]() {
            int new_time = was_paused ? game.window_ticks() : game.ticks();
            return new_time - old_time >= duration;
        });
    };

    auto wait_func = [](Game& game, const sol::protected_function& func) {
        auto& scheduler = game.get_current_scripting_interface()->get_scheduler();
        scheduler.yield([&game, func]() {
            auto result = func();
            if (!result.valid()) {
                sol::error err = result;
                throw err;
            }
            return result.get_type() != sol::type::lua_nil
                && (result.get_type() != sol::type::boolean || result.get<bool>());
        });
    };

    lua["wait"] = sol::yielding(sol::overload(
        [&](int duration) { wait(game, duration); },
        [&](const sol::protected_function& func) { wait_func(game, func); }
    ));

    // Logging
    auto log = lua["logger"].get_or_create<sol::table>();
    log["info"] = [](const std::string& message) {
        LOGGER_I << message;
    };
    log["debug"] = [](const std::string& message) {
        LOGGER_D << message;
    };
    log["warning"] = [](const std::string& message) {
        LOGGER_W << message;
    };
    log["error"] = [](const std::string& message) {
        LOGGER_E << message;
    };

    // Bit operations
    auto bit = lua["bit"].get_or_create<sol::table>();
    bit["bor"] = [](int a, int b) { return a | b; };
    bit["band"] = [](int a, int b) { return a & b; };
    bit["bxor"] = [](int a, int b) { return a ^ b; };
    bit["bnot"] = [](int a) { return ~a; };
    bit["rshift"] = [](int a, int b) { return a >> b; };
    bit["lshift"] = [](int a, int b) { return a << b; };

    // Direction utilities
    auto dir = lua["direction"].get_or_create<sol::table>();
    dir["contains"] = [](int dir, int other) {
        return direction_contains(static_cast<Direction>(dir), static_cast<Direction>(other));
    };
    dir["opposite"] = [](int dir) {
        return static_cast<int>(opposite_direction(static_cast<Direction>(dir)));
    };
    dir["to_vector"] = [](int dir) {
        return direction_to_vector(static_cast<Direction>(dir));
    };
    dir["from_vector"] = [](xd::vec2 vec) {
        return static_cast<int>(vector_to_direction(vec));
    };
    dir["to_string"] = [](int dir) {
        return direction_to_string(static_cast<Direction>(dir));
    };
    dir["from_string"] = [](const std::string& str) {
        return static_cast<int>(string_to_direction(str));
    };
    dir["facing_direction"] = [](xd::vec2 pos1, xd::vec2 pos2, std::optional<bool> diagonal) {
        return static_cast<int>(facing_direction(pos1, pos2, diagonal.value_or(false)));
    };
    dir["is_diagonal"] = [](int dir) {
        return is_diagonal(static_cast<Direction>(dir));
    };
    dir["to_four_directions"] = [](int dir) {
        return diagonal_to_four_directions(static_cast<Direction>(dir));
    };

    // Wait for a command result
    auto result_wait = [&game](Command_Result* cmd) {
        auto& scheduler = game.get_current_scripting_interface()->get_scheduler();
        scheduler.yield(*cmd);
    };

    // Returned from commands that allow yielding
    auto cmd_result_type = lua.new_usertype<Command_Result>("Command_Result");
    cmd_result_type["completed"] = sol::property((bool (Command_Result::*)() const) & Command_Result::is_complete);
    cmd_result_type["stopped"] = sol::property(&Command_Result::is_stopped);
    cmd_result_type["paused"] = sol::property(&Command_Result::is_paused);
    cmd_result_type["is_complete"] = [](Command_Result& result, std::optional<int> ticks) {
        return ticks.has_value() ? result.is_complete(ticks.value()) : result.is_complete();
    };
    cmd_result_type["execute"] = sol::overload(
        (void (Command_Result::*)()) & Command_Result::execute,
        (void (Command_Result::*)(int)) & Command_Result::execute
    );
    cmd_result_type["wait"] = sol::yielding(result_wait);
    cmd_result_type["stop"] = &Command_Result::stop;
    cmd_result_type["force_stop"] = &Command_Result::force_stop;
    cmd_result_type["pause"] = &Command_Result::pause;
    cmd_result_type["resume"] = &Command_Result::resume;

    // Like Command_Result but stores the index of selected choice
    auto choice_result_type = lua.new_usertype<Choice_Result>("Choice_Result",
        sol::base_classes, sol::bases<Command_Result>());
    choice_result_type["selected"] = sol::property(
        [](Choice_Result& cr) { return cr.choice_index() == -1 ? -1 : cr.choice_index() + 1; });

    // A generic command for waiting (used in NPC scheduling)
    lua["Wait_Command"] = [&](int duration, int start_time) {
        return std::make_unique<Command_Result>(std::make_shared<Wait_Command>(
            game,
            duration,
            start_time));
    };
    // A command for moving an object (used in NPC scheduling)
    lua["Move_To_Command"] = [&](Map_Object* obj, float x, float y, bool keep_trying, bool tile_only) {
        return std::make_unique<Command_Result>(std::make_shared<Move_Object_To_Command>(
            *game.get_map(),
            *obj,
            x,
            y,
            tile_only ? Collision_Check_Type::TILE : Collision_Check_Type::BOTH,
            keep_trying));
    };
    // A command for showing text (used in NPC scheduling)
    lua["Text_Command"] = sol::overload(
        [&](Text_Options options, long start_time) {
            auto command = std::make_shared<Show_Text_Command>(game, options);

            if (start_time >= 0) {
                command->set_start_time(start_time);
            }
            return std::make_unique<Command_Result>(command);
        },
        [&](Map_Object* object, const std::string& text, long duration, long start_time) {
            Text_Options options(object);
            options.set_text(text)
                .set_duration(duration)
                .set_position_type(Text_Position_Type::CENTERED_X | Text_Position_Type::BOTTOM_Y);

            auto command = std::make_shared<Show_Text_Command>(game, options);

            if (start_time >= 0) {
                command->set_start_time(start_time);
            }
            return std::make_unique<Command_Result>(command);
        }
    );
    // A command to show an object's pose (used in NPC scheduling)
    lua["Pose_Command"] = [&](Map_Object* object, const std::string& pose, const std::string& state, Direction direction) {
        auto holder_type = Show_Pose_Command::Holder_Type::MAP_OBJECT;
        Show_Pose_Command::Holder_Info holder_info{ holder_type, object->get_id() };
        return std::make_unique<Command_Result>(std::make_shared<Show_Pose_Command>(
            *game.get_map(), holder_info, pose, state, direction));
    };
}
