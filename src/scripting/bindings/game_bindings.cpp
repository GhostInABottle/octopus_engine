#include "../../../include/audio_player.hpp"
#include "../../../include/clock.hpp"
#include "../../../include/configurations.hpp"
#include "../../../include/environments/environment.hpp"
#include "../../../include/game.hpp"
#include "../../../include/save_file.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/utility/file.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>

void bind_game_types(sol::state& lua) {
    // Input type
    lua.new_enum<xd::input_type>("Input_Type",
        {
            {"keyboard", xd::input_type::INPUT_KEYBOARD},
            {"mouse", xd::input_type::INPUT_MOUSE},
            {"gamepad", xd::input_type::INPUT_GAMEPAD}
        }
    );

    // Game object
    auto game_type = lua.new_usertype<Game>("Game");

    game_type["width"] = sol::property(&Game::window_width);
    game_type["height"] = sol::property(&Game::window_height);
    game_type["framebuffer_width"] = sol::property(&Game::framebuffer_width);
    game_type["framebuffer_height"] = sol::property(&Game::framebuffer_height);
    game_type["game_width"] = sol::property([](Game& game) { return game.game_width(); });
    game_type["game_height"] = sol::property([](Game& game) { return game.game_height(); });
    game_type["game_center"] = sol::property([](Game& game) { return game.game_center(); });
    game_type["magnification"] = sol::property(&Game::get_magnification, &Game::set_magnification);

    game_type["ticks"] = sol::property(&Game::ticks);
    game_type["window_ticks"] = sol::property(&Game::window_ticks);
    game_type["fps"] = sol::property(&Game::fps);
    game_type["frame_count"] = sol::property(&Game::frame_count);
    game_type["stopped"] = sol::property(&Game::stopped);
    game_type["seconds"] = sol::property(&Game::seconds);
    game_type["paused"] = sol::property(&Game::is_paused);
    game_type["pausing_enabled"] = sol::property(&Game::is_pausing_enabled, &Game::set_pausing_enabled);
    game_type["script_scheduler_paused"] = sol::property(&Game::is_script_scheduler_paused, &Game::set_script_scheduler_paused);
    game_type["debug"] = sol::property(&Game::is_debug);

    game_type["game_data_filesystem"] = sol::property([](Game&) {
        return file_utilities::game_data_filesystem();
    });
    game_type["user_data_filesystem"] = sol::property([](Game&) {
        return file_utilities::user_data_filesystem();
        });
    game_type["data_folder"] = sol::property([](Game& game) {
        return file_utilities::user_data_folder(game.get_environment());
    });

    game_type["triggered_keys"] = sol::property([](Game* game) { return sol::as_table(game->triggered_keys()); });
    game_type["last_input_type"] = sol::property(&Game::get_last_input_type);
    game_type["gamepad_enabled"] = sol::property(&Game::gamepad_enabled);
    game_type["gamepad_names"] = sol::property([](Game* game) { return sol::as_table(game->gamepad_names()); });
    game_type["gamepad_name"] = sol::property([](Game* game) { return game->get_gamepad_name(-1); });
    game_type["gamepad_guid"] = sol::property([](Game* game) { return game->get_gamepad_guid(-1); });
    game_type["character_input"] = sol::property(&Game::character_input);

    game_type["monitor_resolution"] = sol::property(&Game::get_current_resolution);
    game_type["monitor_resolutions"] = sol::property([&](Game& game) {
        return sol::as_table(game.get_monitor_resolutions());
    });
    game_type["fullscreen"] = sol::property(&Game::is_fullscreen, &Game::set_fullscreen);

    game_type["command_line_args"] = sol::property([&](Game& game) {
        return sol::as_table(game.get_command_line_args());
    });
    game_type["audio_player"] = sol::property(&Game::get_audio_player);
    game_type["environment"] = sol::property(&Game::get_environment);

    game_type["set_size"] = &Game::set_window_size;

    game_type["exit"] = &Game::exit;
    game_type["pause"] = &Game::pause;
    game_type["resume"] = [](Game* game, std::optional<std::string> script) {
        game->resume(script.value_or(""));
    };

    game_type["pressed"] = [](Game* game, const std::string& key) { return game->pressed(key); };
    game_type["triggered"] = sol::overload(
        [](Game* game) { return game->triggered(); },
        [](Game* game, const std::string& key) { return game->triggered(key); }
    );
    game_type["triggered_once"] = [](Game* game, const std::string& key) { return game->triggered_once(key); };
    game_type["bind_key"] = sol::resolve<void(const std::string&, const std::string&)>(&Game::bind_key);
    game_type["unbind_physical_key"] = sol::resolve<void(const std::string&)>(&Game::unbind_physical_key);
    game_type["unbind_virtual_key"] = &Game::unbind_virtual_key;
    game_type["get_bound_keys"] = [](Game* game, const std::string& virtual_name) {
        return sol::as_table(game->get_bound_keys(virtual_name));
    };
    game_type["begin_character_input"] = &Game::begin_character_input;
    game_type["end_character_input"] = &Game::end_character_input;
    game_type["get_key_name"] = &Game::get_key_name;
    game_type["get_gamepad_name"] = &Game::get_gamepad_name;
    game_type["get_gamepad_guid"] = &Game::get_gamepad_guid;

    game_type["run_script"] = &Game::run_script;
    game_type["run_script_file"] = &Game::run_script_file;
    game_type["run_function"] = &Game::run_function;
    game_type["reset_scripting"] = &Game::reset_scripting;

    game_type["stop_time"] = [](Game* game) { game->get_clock()->stop_time(); };
    game_type["resume_time"] = [](Game* game) { game->get_clock()->resume_time(); };

    game_type["load_map"] = sol::overload(
        [](Game* game, const std::string& filename) {
            game->set_next_map(filename);
        },
        [](Game* game, const std::string& filename, int dir) {
            game->set_next_map(filename, static_cast<Direction>(dir));
        },
        [](Game* game, const std::string& filename, int dir,
                std::optional<std::string> music, std::optional<std::string> layer) {
            game->set_next_map(filename, static_cast<Direction>(dir), std::nullopt, music, layer);
        },
        [](Game* game, const std::string& filename, float x, float y, int dir) {
            game->set_next_map(filename, static_cast<Direction>(dir), xd::vec2{ x, y });
        },
        [](Game* game, const std::string& filename, xd::vec2 pos, int dir,
                std::optional<std::string> music, std::optional<std::string> layer) {
            game->set_next_map(filename, static_cast<Direction>(dir), pos, music, layer);
        }
    );

    game_type["get_config"] = [](Game*, const std::string& key) { return Configurations::get_string(key); };
    game_type["get_bool_config"] = [](Game*, const std::string& key) { return Configurations::get<bool>(key); };
    game_type["get_float_config"] = [](Game*, const std::string& key) { return Configurations::get<float>(key); };
    game_type["get_int_config"] = [](Game*, const std::string& key) { return Configurations::get<int>(key); };
    game_type["get_unsigned_config"] = [](Game*, const std::string& key) { return Configurations::get<unsigned int>(key); };
    game_type["get_string_config"] = [](Game*, const std::string& key) { return Configurations::get<std::string>(key); };
    game_type["set_bool_config"] = [](Game*, const std::string& key, bool value) {
        Configurations::set(key, value);
    };
    game_type["set_float_config"] = [](Game*, const std::string& key, float value) {
        Configurations::set(key, value);
    };
    game_type["set_int_config"] = [](Game*, const std::string& key, int value) {
        Configurations::set(key, value);
    };
    game_type["set_unsigned_config"] = [](Game*, const std::string& key, unsigned int value) {
        Configurations::set(key, value);
    };
    game_type["set_string_config"] = [](Game*, const std::string& key, const std::string& value) {
        // Log level is usually changed to suppress logs, so don't log it
        bool should_log = key != "logging.level";
        Configurations::set(key, value, should_log);
    };

    game_type["wait_for_input"] = sol::yielding(sol::overload(
        [](Game* game, const std::string& key) {
            auto& scheduler = game->get_current_scripting_interface()->get_scheduler();
            scheduler.yield([game, key]() {
                return game->triggered(key);
                });
        },
        [](Game* game) {
            auto& scheduler = game->get_current_scripting_interface()->get_scheduler();
            scheduler.yield([game]() {
                return game->triggered();
                });
        }
    ));

    game_type["text_width"] = [](Game& game, const std::string& text) {
        return game.text_width(text);
    };

    game_type["load_lua_file"] = [&lua](Game&, const std::string& filename) {
        auto filesystem = file_utilities::game_data_filesystem();
        if (!filesystem->exists(filename)) {
            throw std::runtime_error{ "File was not found while trying to load Lua file: " + filename };
        }
        auto script = filesystem->read_file(filename);

        // The @ indicates a filename for debugging and printing the stack trace
        sol::load_result result = lua.load(script, "@" + filename);
        if (!result.valid()) {
            throw std::runtime_error("Error loading " + filename + ": " + result.get<std::string>());
        }

        sol::protected_function f = result;
        return f;
    };

    game_type["open_url"] = [](Game&, const std::string& url) {
        return file_utilities::open_url(url);
    };
}
