#include "../../include/scripting/scripting_interface.hpp"
#include "../../include/scripting/script_bindings.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/map.hpp"
#include "../../include/map_object.hpp"
#include "../../include/command.hpp"
#include "../../include/configurations.hpp"
#include "../../include/log.hpp"
#include "../../include/utility/file.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/xd/lua.hpp"

namespace detail {
    static int require_lua_file(lua_State* state) {
        auto require_path = sol::stack::get<std::string>(state);
        auto filename{require_path};
        // Lua supports . as a platform-agnostic directory separator
        string_utilities::replace_all(filename, ".", "/");
        filename += ".lua";

        auto filesystem = file_utilities::game_data_filesystem();
        if (!filesystem->file_exists(filename)) {
            // Push error string and return its stack index
            sol::stack::push(state, " file does not exist: " + filename);
            return 1;
        }

        try {
            auto script = filesystem->read_file(filename);
            // Load and push the code (or error message) to the stack, and return the index
            luaL_loadbuffer(state, script.data(), script.size(), ("@" + filename).c_str());
        } catch (std::exception& ex) {
            sol::stack::push(state, " error while reading " + filename + ": " + ex.what());
        }

        return 1;
    }
}

Game* Scripting_Interface::game = nullptr;

Scripting_Interface::Scripting_Interface(Game& game) : scheduler(*game.get_lua_vm()) {
    if (!Scripting_Interface::game) {
        Scripting_Interface::game = &game;
        setup_scripts();
    }
}

void Scripting_Interface::update() {
    // Execute pending commands
    auto current_map = game->get_map();
    for (auto i = commands.begin(); i < commands.end();) {
        auto& command = *i;

        // Don't execute commands if they belong to a different map
        auto command_map = command->get_map_ptr();
        auto map_changed = command_map && command_map != current_map;

        if (!map_changed) {
            command->execute();
        }

        auto command_complete = command->is_complete();
        if (map_changed || command_complete) {
            if (!command_complete) {
                // If map changed, mark the command as complete to let any Lua
                // results waiting on it know that it's done
                command->force_stop();
            }
            i = commands.erase(i);
            continue;
        }
        ++i;
    }
    if (scheduler.pending_tasks() > 0)
        scheduler.run();
}

void Scripting_Interface::schedule_code(const std::string& script, const std::string& context) {
    if (script.empty()) {
        LOGGER_W << "Tried to schedule an empty script";
        return;
    };
    scheduler.start(script, context);
}

void Scripting_Interface::schedule_file(const std::string& filename, const std::string& context) {
    auto fs = file_utilities::game_data_filesystem();
    scheduler.start(fs->read_file(filename), context, "@" + filename);
}

void Scripting_Interface::schedule_function(const sol::protected_function& function, const std::string& context) {
    scheduler.start(function, context);
}

void Scripting_Interface::set_globals() {
    auto& vm = *game->get_lua_vm();
    // Global variables
    vm.globals()["game"] = game;
    vm.globals()["current_map"] = game->get_map();
    vm.globals()["camera"] = game->get_camera();
    vm.globals()["player"] = game->get_player();
    // Directions
    vm.globals()["UP"] = static_cast<int>(Direction::UP);
    vm.globals()["RIGHT"] = static_cast<int>(Direction::RIGHT);
    vm.globals()["DOWN"] = static_cast<int>(Direction::DOWN);
    vm.globals()["LEFT"] = static_cast<int>(Direction::LEFT);
    vm.globals()["FORWARD"] = static_cast<int>(Direction::FORWARD);
    vm.globals()["BACKWARD"] = static_cast<int>(Direction::BACKWARD);
}

sol::state& Scripting_Interface::lua_state() {
    return game->get_lua_vm()->lua_state();
}

void Scripting_Interface::setup_scripts() {
    auto& vm = *game->get_lua_vm();

    if (Configurations::get<bool>("debug.seed-lua-rng")) {
        scheduler.start("math.randomseed(os.time())", "");
    }

    auto& lua = vm.lua_state();

    // Use custom searcher for require
    lua["package"]["searchers"] = lua.create_table_with(1, detail::require_lua_file);

    // Bind the Lua engine interface

    bind_utility_types(lua, *game);

    bind_file_types(lua, *game);

    bind_text_types(lua, *game);

    bind_math_types(lua);

    bind_map_object_types(lua, *game);

    bind_audio_types(lua, *game);

    bind_game_types(lua, *game);

    bind_layer_types(lua, *game);

    bind_map_types(lua);

    bind_camera_types(lua, *game);

    bind_canvas_types(lua, *game);
}
