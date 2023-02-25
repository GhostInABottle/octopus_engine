#include "../../include/scripting/scripting_interface.hpp"
#include "../../include/scripting/bindings/canvas_bindings.hpp"
#include "../../include/scripting/bindings/game_bindings.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/map.hpp"
#include "../../include/collision_record.hpp"
#include "../../include/image_layer.hpp"
#include "../../include/object_layer.hpp"
#include "../../include/map_object.hpp"
#include "../../include/command_result.hpp"
#include "../../include/commands.hpp"
#include "../../include/text_parser.hpp"
#include "../../include/utility/color.hpp"
#include "../../include/utility/direction.hpp"
#include "../../include/utility/file.hpp"
#include "../../include/configurations.hpp"
#include "../../include/sprite_data.hpp"
#include "../../include/log.hpp"
#include "../../include/xd/audio.hpp"
#include "../../include/xd/lua.hpp"
#include <sstream>
#include <ctime>

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
    scheduler.start_file(filename, context);
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
    auto result_wait = [](Command_Result* cmd) {
        auto& scheduler = game->get_current_scripting_interface()->scheduler;
        scheduler.yield(*cmd);
    };
    auto wait = [](Game& game, int duration) {
        bool was_paused = game.is_paused();
        int old_time = was_paused ? game.window_ticks() : game.ticks();
        auto& scheduler = game.get_current_scripting_interface()->scheduler;
        scheduler.yield([&game, was_paused, old_time, duration]() {
            int new_time = was_paused ? game.window_ticks() : game.ticks();
            return new_time - old_time >= duration;
        });
    };

    auto wait_func = [](Game& game, const sol::protected_function& func) {
        auto& scheduler = game.get_current_scripting_interface()->scheduler;
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

    auto& lua = vm.lua_state();

    lua["wait"] = sol::yielding(sol::overload(
        [&](int duration) { wait(*game, duration); },
        [&](const sol::protected_function& func) { wait_func(*game, func); }
    ));

    auto tm_type = lua.new_usertype<std::tm>("Calendar_Time");
    tm_type["second"] = sol::readonly(&std::tm::tm_sec);
    tm_type["minute"] = sol::readonly(&std::tm::tm_min);
    tm_type["hour"] = sol::readonly(&std::tm::tm_hour);
    tm_type["month_day"] = sol::readonly(&std::tm::tm_mday);
    tm_type["month"] = sol::readonly(&std::tm::tm_mon);
    tm_type["year"] = sol::property([](std::tm& val) { return val.tm_year + 1900; });
    tm_type["week_day"] = sol::readonly(&std::tm::tm_wday);
    tm_type["year_day"] = sol::readonly(&std::tm::tm_yday);
    tm_type["is_dst"] = sol::property([](std::tm& val) {
        return val.tm_isdst < 0 ? std::nullopt : std::optional<bool>{ val.tm_isdst > 0 };
    });

    auto info_type = lua.new_usertype<file_utilities::Path_Info>("Path_Info");
    info_type["name"] = sol::readonly(&file_utilities::Path_Info::name);
    info_type["is_regular"] = sol::readonly(&file_utilities::Path_Info::is_regular);
    info_type["is_directory"] = sol::readonly(&file_utilities::Path_Info::is_directory);
    info_type["timestamp"] = sol::readonly(&file_utilities::Path_Info::timestamp);
    info_type["calendar_time"] = sol::readonly(&file_utilities::Path_Info::calendar_time);

    auto filesystem = lua["filesystem"].get_or_create<sol::table>();
    filesystem["exists"] = &file_utilities::file_exists;
    filesystem["is_regular_file"] = &file_utilities::is_regular_file;
    filesystem["is_directory"] = &file_utilities::is_directory;
    filesystem["list_directory"] = &file_utilities::directory_content_names;
    filesystem["list_detailed_directory"] = &file_utilities::directory_content_details;
    filesystem["copy"] = &file_utilities::copy_file;
    filesystem["remove"] = &file_utilities::remove_file;
    filesystem["is_absolute"] = &file_utilities::is_absolute_path;
    filesystem["get_basename"] = &file_utilities::get_filename_component;
    filesystem["get_stem"] = &file_utilities::get_stem_component;
    filesystem["last_write_time"] = &file_utilities::last_write_time;

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

    // Returned from commands that allow yielding
    auto cmd_result_type = lua.new_usertype<Command_Result>("Command_Result");
    cmd_result_type["completed"] = sol::property((bool (Command_Result::*)() const) &Command_Result::is_complete);
    cmd_result_type["stopped"] = sol::property(&Command_Result::is_stopped);
    cmd_result_type["paused"] = sol::property(&Command_Result::is_paused);
    cmd_result_type["is_complete"] = sol::overload(
        &Command_Result::operator(),
        (bool (Command_Result::*)(int) const) &Command_Result::is_complete
    );
    cmd_result_type["execute"] = sol::overload(
        (void (Command_Result::*)()) &Command_Result::execute,
        (void (Command_Result::*)(int)) &Command_Result::execute
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
            *game,
            duration,
            start_time));
    };
    // A command for moving an object (used in NPC scheduling)
    lua["Move_To_Command"] = [&](Map_Object* obj, float x, float y, bool keep_trying, bool tile_only) {
        return std::make_unique<Command_Result>(std::make_shared<Move_Object_To_Command>(
            *game->get_map(),
            *obj,
            x,
            y,
            tile_only ? Collision_Check_Type::TILE : Collision_Check_Type::BOTH,
            keep_trying));
    };
    // A command for showing text (used in NPC scheduling)
    lua["Text_Command"] = sol::overload(
        [&](Text_Options options, long start_time) {
            auto command = std::make_shared<Show_Text_Command>(*game, options);

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

            auto command = std::make_shared<Show_Text_Command>(*game, options);

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
            *game->get_map(), holder_info, pose, state, direction));
    };

    // Text positioning
    lua.new_enum<Text_Position_Type>("Text_Position_Type",
        {
            {"none", Text_Position_Type::NONE},
            {"exact_x", Text_Position_Type::EXACT_X},
            {"centered_x", Text_Position_Type::CENTERED_X},
            {"exact_y", Text_Position_Type::EXACT_Y},
            {"bottom_y", Text_Position_Type::BOTTOM_Y},
            {"camera_relative", Text_Position_Type::CAMERA_RELATIVE},
            {"always_visible", Text_Position_Type::ALWAYS_VISIBLE}
        });

    // Options for displaying text
    auto options_type = lua.new_usertype<Text_Options>("Text_Options",
        sol::call_constructor, sol::constructors<Text_Options(), Text_Options(Map_Object*), Text_Options(xd::vec2)>());
    options_type["text"] = sol::readonly(&Text_Options::text);
    options_type["choices"] = sol::readonly(&Text_Options::choices);
    options_type["object"] = sol::readonly(&Text_Options::object);
    options_type["position"] = sol::readonly(&Text_Options::position);
    options_type["position_type"] = sol::readonly(&Text_Options::position_type);
    options_type["duration"] = sol::readonly(&Text_Options::duration);
    options_type["centered"] = sol::readonly(&Text_Options::centered);
    options_type["show_dashes"] = sol::readonly(&Text_Options::show_dashes);
    options_type["cancelable"] = sol::readonly(&Text_Options::cancelable);
    options_type["choice_indent"] = sol::readonly(&Text_Options::choice_indent);
    options_type["translated"] = sol::readonly(&Text_Options::translated);
    options_type["canvas_priority"] = sol::readonly(&Text_Options::canvas_priority);
    options_type["fade_in_duration"] = sol::readonly(&Text_Options::fade_in_duration);
    options_type["fade_out_duration"] = sol::readonly(&Text_Options::fade_out_duration);
    options_type["background_visible"] = sol::readonly(&Text_Options::background_visible);
    options_type["background_color"] = sol::readonly(&Text_Options::background_color);
    options_type["set_text"] = &Text_Options::set_text;
    options_type["set_object"] = &Text_Options::set_object;
    options_type["set_position"] = &Text_Options::set_position;
    options_type["set_duration"] = &Text_Options::set_duration;
    options_type["set_centered"] = &Text_Options::set_centered;
    options_type["set_show_dashes"] = &Text_Options::set_show_dashes;
    options_type["set_translated"] = &Text_Options::set_translated;
    options_type["set_cancelable"] = &Text_Options::set_cancelable;
    options_type["set_choice_indent"] = &Text_Options::set_choice_indent;
    options_type["set_canvas_priority"] = &Text_Options::set_canvas_priority;
    options_type["set_fade_in_duration"] = &Text_Options::set_fade_in_duration;
    options_type["set_fade_out_duration"] = &Text_Options::set_fade_out_duration;
    options_type["set_background_visible"] = &Text_Options::set_background_visible;
    options_type["set_background_color"] = &Text_Options::set_background_color;
    options_type["set_choices"] = [&](Text_Options& options, const sol::table& table) -> Text_Options& {
        std::vector<std::string> choices;
        for (auto& kv : table) {
            choices.push_back(kv.second.as<std::string>());
        }
        return options.set_choices(choices);
    };
    options_type["set_position_type"] = &Text_Options::set_position_type;

    // Show text
    lua["text"] = sol::overload(
        [&](const Text_Options& options) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(*game, options);
        },
        [&](Map_Object& obj, const std::string& text) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                *game, Text_Options{ &obj }.set_text(text));
        },
        [&](xd::vec2& position, const std::string& text) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                *game, Text_Options{ position }.set_text(text));
        },
        [&](Map_Object& obj, const std::string& text, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                *game, Text_Options{ &obj }.set_text(text).set_duration(duration));
        },
        [&](xd::vec2& position, const std::string& text, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                *game, Text_Options{ position }.set_text(text).set_duration(duration));
        }
    );
    lua["centered_text"] = sol::overload(
        [&](float y, const std::string& text) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                *game, Text_Options{ xd::vec2{0.0f, y} }.set_text(text).set_centered(true));
        },
        [&](float y, const std::string& text, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                *game, Text_Options{ xd::vec2{0.0f, y} }.set_text(text).set_duration(duration).set_centered(true));
        }
    );

    // Show some text followed by a list of choices
    auto set_options = [](Text_Options& options, const std::vector<std::string>& choices,
        const std::string& text, std::optional<bool> cancelable) {
            options.set_choices(choices).set_text(text).set_cancelable(cancelable.value_or(false));
    };
    lua["choices"] = sol::overload(
        [&](const Text_Options& options) {
            auto si = game->get_current_scripting_interface();
            return si->register_choice_command<Show_Text_Command>(*game, options);
        },
        [&](Map_Object& obj, const std::string& text, const sol::table& table, std::optional<bool> cancelable) {
            std::vector<std::string> choices;
            for (auto& kv : table) {
                choices.push_back(kv.second.as<std::string>());
            }
            Text_Options options{&obj};
            set_options(options, choices, text, cancelable);
            auto si = game->get_current_scripting_interface();
            return si->register_choice_command<Show_Text_Command>(
                *game, std::move(options));
        },
        [&](xd::vec2& position, const std::string& text, const sol::table& table, std::optional<bool> cancelable) {
            std::vector<std::string> choices;
            for (auto& kv : table) {
                choices.push_back(kv.second.as<std::string>());
            }
            Text_Options options{position};
            set_options(options, choices, text, cancelable);
            auto si = game->get_current_scripting_interface();
            return si->register_choice_command<Show_Text_Command>(
                *game, std::move(options));
        }
    );

    // 2D vector
    auto vec2_type = lua.new_usertype<xd::vec2>("Vec2",
        sol::call_constructor, sol::constructors<xd::vec2(), xd::vec2(const xd::vec2&), xd::vec2(float, float)>());
    vec2_type["x"] = &xd::vec2::x;
    vec2_type["y"] = &xd::vec2::y;
    vec2_type["length"] = [](xd::vec2& v) { return xd::length(v); };
    vec2_type["normal"] = [](xd::vec2& v) { return xd::normalize(v); };
    vec2_type[sol::meta_function::addition] = [](const xd::vec2& v1, const xd::vec2& v2) { return v1 + v2; };
    vec2_type[sol::meta_function::subtraction] = [](const xd::vec2& v1, const xd::vec2& v2) { return v1 - v2; };
    vec2_type[sol::meta_function::multiplication] = sol::overload(
        [](const xd::vec2& v1, float f) { return v1 * f; },
        [](float f, const xd::vec2& v1) { return f * v1; }
    );
    vec2_type[sol::meta_function::to_string] = [](const xd::vec2& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(1);
        ss << "Vec2(" << val.x << ", " << val.y << ")";
        return ss.str();
    };

    // 3D vector
    auto vec3_type = lua.new_usertype<xd::vec3>("Vec3",
        sol::call_constructor, sol::constructors<xd::vec3(), xd::vec3(const xd::vec3&), xd::vec3(float, float, float)>());
    vec3_type["x"] = &xd::vec3::x;
    vec3_type["y"] = &xd::vec3::y;
    vec3_type["z"] = &xd::vec3::z;
    vec3_type["length"] = [](xd::vec3& v) { return xd::length(v); };
    vec3_type["normal"] = [](xd::vec3& v) { return xd::normalize(v); };
    vec3_type[sol::meta_function::addition] = [](const xd::vec3& v1, const xd::vec3& v2) { return v1 + v2; };
    vec3_type[sol::meta_function::subtraction] = [](const xd::vec3& v1, const xd::vec3& v2) { return v1 - v2; };
    vec3_type[sol::meta_function::multiplication] = sol::overload(
        [](const xd::vec3& v1, float f) { return v1 * f; },
        [](float f, const xd::vec3& v1) { return f * v1; }
    );
    vec3_type[sol::meta_function::to_string] = [](const xd::vec3& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(1);
        ss << "Vec3(" << val.x << ", " << val.y << ", " << val.z << ")";
        return ss.str();
    };

    // 4D vector (or color)
    auto vec4_type = lua.new_usertype<xd::vec4>("Vec4",
        sol::call_constructor, sol::constructors<xd::vec4(), xd::vec4(const xd::vec4&), xd::vec4(float, float, float, float)>());
    vec4_type["x"] = &xd::vec4::x;
    vec4_type["y"] = &xd::vec4::y;
    vec4_type["z"] = &xd::vec4::z;
    vec4_type["w"] = &xd::vec4::w;
    vec4_type["r"] = &xd::vec4::r;
    vec4_type["g"] = &xd::vec4::g;
    vec4_type["b"] = &xd::vec4::b;
    vec4_type["a"] = &xd::vec4::a;
    vec4_type["length"] = [](const xd::vec4& v) { return xd::length(v); };
    vec4_type["normal"] = [](const xd::vec4& v) { return xd::normalize(v); };
    vec4_type[sol::meta_function::addition] = [](const xd::vec4& v1, const xd::vec4& v2) { return v1 + v2; };
    vec4_type[sol::meta_function::subtraction] = [](const xd::vec4& v1, const xd::vec4& v2) { return v1 - v2; };
    vec4_type[sol::meta_function::multiplication] = sol::overload(
        [](const xd::vec4& v1, float f) { return v1 * f; },
        [](float f, const xd::vec4& v1) { return f * v1; }
    );
    vec4_type[sol::meta_function::to_string] = [](const xd::vec4& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(1);
        ss << "Vec4(" << val.x << ", " << val.y << ", " << val.z << ", " << val.w << ")";
        return ss.str();
    };
    vec4_type["to_hex"] = [](const xd::vec4& color) { return color_to_hex(color); };

    // Aliases for creating a color
    lua["Color"] = sol::overload(
        [](xd::vec4 other) { return xd::vec4{ other }; },
        [](float r, float g, float b, float a) { return xd::vec4{ r, g, b, a }; },
        [](float r, float g, float b) { return xd::vec4{ r, g, b, 1.0f }; },
        [](std::string name) {
            if (name == "clear") {
                auto clear = hex_to_color(Configurations::get<std::string>("startup.clear-color"));
                return xd::vec4{ clear };
            } else if (name == "none") {
                return xd::vec4{};
            } else if (name == "black") {
                return xd::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
            } else if (name == "red") {
                return xd::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };
            } else if (name == "green") {
                return xd::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
            } else if (name == "blue") {
                return xd::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };
            } else if (name == "yellow") {
                return xd::vec4{ 1.0f, 1.0f, 0.0f, 1.0f };
            } else if (name == "white") {
                return xd::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
            }

            // Throws if name is not a valid hex
            return xd::vec4{ hex_to_color(name) };
        }
    );
    // Rectangle
    auto rect_type = lua.new_usertype<xd::rect>("Rect",
        sol::call_constructor, sol::constructors<
        xd::rect(),
        xd::rect(const xd::rect&),
        xd::rect(float, float, float, float),
        xd::rect(const xd::vec2&, const xd::vec2&),
        xd::rect(const xd::vec2&, float, float),
        xd::rect(const xd::vec4&)>());
    rect_type["x"] = &xd::rect::x;
    rect_type["y"] = &xd::rect::y;
    rect_type["w"] = &xd::rect::w;
    rect_type["h"] = &xd::rect::h;
    rect_type["position"] = sol::property(sol::resolve<xd::vec2() const>(&xd::rect::position),
        sol::resolve<void (xd::vec2)>(&xd::rect::position));
    rect_type["size"] = sol::property(sol::resolve<xd::vec2() const>(&xd::rect::size),
        sol::resolve<void(xd::vec2)>(&xd::rect::size));
    rect_type["intersects"] = &xd::rect::intersects;
    rect_type[sol::meta_function::to_string] = [](const xd::rect& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(0);
        ss << "Rect (" << val.x << ", " << val.y << ") " << val.w << "x" << val.h;
        return ss.str();
    };

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
            obj->set_sprite(*game, filename);
        });
    object_type["reset"] = &Map_Object::reset;
    object_type["set_sprite"] = [&](Map_Object* obj, const std::string& filename, std::optional<std::string> pose) {
        obj->set_sprite(*game, filename, pose.value_or(""));
    };
    object_type["show_pose"] = [&](Map_Object* obj, const std::string& pose_name, std::optional<std::string> state, std::optional<Direction> dir) {
        auto si = game->get_current_scripting_interface();
        auto holder_type = Show_Pose_Command::Holder_Type::MAP_OBJECT;
        Show_Pose_Command::Holder_Info holder_info{ holder_type, obj->get_id() };
        return si->register_command<Show_Pose_Command>(
            *game->get_map(), holder_info, pose_name, state.value_or(""), dir.value_or(Direction::NONE));
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
    object_type["overrides_tile_collision"] = sol::property(&Map_Object::overrides_tile_collision, &Map_Object::set_override_tile_collision);
    object_type["strict_multidirectional_movement"] = sol::property(&Map_Object::get_strict_multidirectional_movement, &Map_Object::set_strict_multidirectional_movement);
    object_type["uses_layer_color"] = sol::property(&Map_Object::uses_layer_color, &Map_Object::set_use_layer_color);
    object_type["player_facing"] = sol::property(&Map_Object::is_player_facing, &Map_Object::set_player_facing);
    object_type["triggered_object"] = sol::property(&Map_Object::get_triggered_object, &Map_Object::set_triggered_object);
    object_type["collision_object"] = sol::property(&Map_Object::get_collision_object, &Map_Object::set_collision_object);
    object_type["collision_area"] = sol::property(&Map_Object::get_collision_area, &Map_Object::set_collision_area);
    object_type["outlined"] = sol::property(&Map_Object::is_outlined, &Map_Object::set_outlined);
    object_type["outline_conditions"] = sol::property(&Map_Object::get_outline_conditions, &Map_Object::set_outline_conditions);
    object_type["outline_color"] = sol::property(&Map_Object::get_outline_color, &Map_Object::set_outline_color);
    object_type["outlined_object"] = sol::property(
        [&](Map_Object* object) {
            auto id = object->get_outlined_object_id();
            return id > -1 ? game->get_map()->get_object(id) : nullptr;
        },
        [&](Map_Object* object, Map_Object* other) {
            auto map = game->get_map();
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
    object_type["centered_position"] = sol::property(&Map_Object::get_centered_position);
    object_type["bounding_box"] = sol::property(&Map_Object::get_bounding_box);
    object_type["speed"] = sol::property(&Map_Object::get_speed, &Map_Object::set_speed);
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
    object_type["sfx_attenuation"] = sol::property(&Map_Object::is_sound_attenuation_enabled, &Map_Object::set_sound_attenuation_enabled);
    object_type["get_property"] = &Map_Object::get_property;
    object_type["set_property"] = &Map_Object::set_property;
    object_type["move_to"] = sol::overload(
        [&](Map_Object* obj, float x, float y) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                    *game->get_map(), *obj, x, y);
        },
        [&](Map_Object* obj, xd::vec2 pos) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                    *game->get_map(), *obj, pos.x, pos.y);
        },
        [&](Map_Object* obj, float x, float y, bool keep_trying) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                    *game->get_map(), *obj, x, y,
                    Collision_Check_Type::BOTH, keep_trying);
        },
        [&](Map_Object* obj, xd::vec2 pos, bool keep_trying) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Move_Object_To_Command>(
                    *game->get_map(), *obj, pos.x, pos.y,
                    Collision_Check_Type::BOTH, keep_trying);
        }
    );
    object_type["move"] = [&](Map_Object* obj, int dir, float pixels, std::optional<bool> skip, std::optional<bool> change_facing) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Move_Object_Command>(
            *game, *obj, static_cast<Direction>(dir), pixels,
            skip.value_or(true), change_facing.value_or(true));
    };
    object_type["update_opacity"] = [&](Map_Object* obj, float opacity, long duration) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
            *game, *obj, opacity, duration);
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

    // Sound effect
    auto sound = lua.new_usertype<xd::sound>("Sound",
        sol::call_constructor, sol::factories(
        [&](const std::string& filename) {
            auto group_type = game->get_sound_group_type();
            return std::make_unique<xd::sound>(*game->get_audio(), filename, group_type);
        },
        [&](const std::string& filename, bool pausable) {
            auto group_type = pausable
                ? channel_group_type::sound
                : channel_group_type::non_pausable_sound;
            return std::make_unique<xd::sound>(*game->get_audio(), filename, group_type);
        }
    ));
    sound["playing"] = sol::property(&xd::sound::playing);
    sound["paused"] = sol::property(&xd::sound::paused);
    sound["stopped"] = sol::property(&xd::sound::stopped);
    sound["offset"] = sol::property(&xd::sound::get_offset, &xd::sound::set_offset);
    sound["volume"] = sol::property(&xd::sound::get_volume, &xd::sound::set_volume);
    sound["pitch"] = sol::property(&xd::sound::get_pitch, &xd::sound::set_pitch);
    sound["looping"] = sol::property(&xd::sound::looping, &xd::sound::set_looping);
    sound["filename"] = sol::property(&xd::sound::get_filename);
    sound["play"] = &xd::sound::play;
    sound["pause"] = &xd::sound::pause;
    sound["stop"] = &xd::sound::stop;
    sound["set_loop_points"] = &xd::sound::set_loop_points;

    // Background music
    auto music = lua.new_usertype<xd::music>("Music",
        sol::call_constructor, sol::factories(
        [&](const std::string& filename) {
            return std::make_shared<xd::music>(*game->get_audio(), filename);
        }
    ));
    music["playing"] = sol::property(&xd::music::playing);
    music["paused"] = sol::property(&xd::music::paused);
    music["stopped"] = sol::property(&xd::music::stopped);
    music["offset"] = sol::property(&xd::music::get_offset, &xd::music::set_offset);
    music["volume"] = sol::property(&xd::music::get_volume, &xd::music::set_volume);
    music["pitch"] = sol::property(&xd::music::get_pitch, &xd::music::set_pitch);
    music["looping"] = sol::property(&xd::music::looping, &xd::music::set_looping);
    music["filename"] = sol::property(&xd::music::get_filename);
    music["play"] = &xd::music::play;
    music["pause"] = &xd::music::pause;
    music["stop"] = &xd::music::stop;
    music["set_loop_points"] = &xd::music::set_loop_points;
    music["fade"] = [&](xd::music* music, float volume, long duration) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Fade_Music_Command>(*game, volume, duration);
    };

    // Input type
    lua.new_enum<xd::input_type>("Input_Type",
        {
            {"keyboard", xd::input_type::INPUT_KEYBOARD},
            {"mouse", xd::input_type::INPUT_MOUSE},
            {"gamepad", xd::input_type::INPUT_GAMEPAD}
        }
    );

    // Game object
    bind_game_types(lua, *game);

    // Map layer
    auto layer_type = lua.new_usertype<Layer>("Layer");
    layer_type["name"] = sol::readonly(&Layer::name);
    layer_type["visible"] = &Layer::visible;
    layer_type["opacity"] = &Layer::opacity;
    layer_type["get_property"] = &Layer::get_property;
    layer_type["set_property"] = &Layer::set_property;
    layer_type["update_opacity"] = [&](Layer* layer, float opacity, long duration) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
                *game, *layer, opacity, duration);
    };

    // Image layer
    auto image_layer_type = lua.new_usertype<Image_Layer>("Image_Layer");
    image_layer_type["name"] = sol::readonly(&Image_Layer::name);
    image_layer_type["visible"] = &Image_Layer::visible;
    image_layer_type["opacity"] = &Image_Layer::opacity;
    image_layer_type["velocity"] = &Image_Layer::velocity;
    image_layer_type["sprite"] = sol::property(
        &Image_Layer::get_sprite_filename,
        [&](Image_Layer* layer, const std::string& filename) {
            layer->set_sprite(*game, filename);
        });
    image_layer_type["get_property"] = &Image_Layer::get_property;
    image_layer_type["set_property"] = &Image_Layer::set_property;
    image_layer_type["update_opacity"] = [&](Image_Layer* layer, float opacity, long duration) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
            *game, *layer, opacity, duration);
    };
    image_layer_type["reset"] = &Image_Layer::reset;
    image_layer_type["set_sprite"] = [&](Image_Layer* layer, const std::string& filename, std::optional<std::string> pose) {
        layer->set_sprite(*game, filename, pose.value_or(""));
    };
    image_layer_type["show_pose"] = [&](Image_Layer* layer, const std::string& pose_name, std::optional<std::string> state, std::optional<Direction> dir) {
        auto si = game->get_current_scripting_interface();
        auto holder_type = Show_Pose_Command::Holder_Type::LAYER;
        Show_Pose_Command::Holder_Info holder_info{ holder_type, layer->id};
        return si->register_command<Show_Pose_Command>(*game->get_map(),
            holder_info, pose_name, state.value_or(""), dir.value_or(Direction::NONE));
    };

    // Object layer
    auto object_layer_type = lua.new_usertype<Object_Layer>("Layer");
    object_layer_type["name"] = sol::readonly(&Object_Layer::name);
    object_layer_type["visible"] = &Object_Layer::visible;
    object_layer_type["opacity"] = &Object_Layer::opacity;
    object_layer_type["tint_color"] = &Object_Layer::tint_color;
    object_layer_type["objects"] = sol::property([&](Object_Layer* layer) {
        return sol::as_table(layer->objects);
    });
    object_layer_type["get_property"] = &Object_Layer::get_property;
    object_layer_type["set_property"] = &Object_Layer::set_property;
    object_layer_type["update_opacity"] = [&](Object_Layer* layer, float opacity, long duration) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
            *game, *layer, opacity, duration);
    };

    // A data bag for defining custom properties on maps/canvases
    auto lua_object_type = lua.new_usertype<Lua_Object>("Lua_Object");
    lua_object_type[sol::meta_function::index] = &Lua_Object::get_lua_property;
    lua_object_type[sol::meta_function::new_index] = &Lua_Object::set_lua_property;

    // Current map / scene
    auto map_type = lua.new_usertype<Map>("Map");
    map_type["data"] = sol::property(&Map::get_lua_data);
    map_type["width"] = sol::property(&Map::get_width);
    map_type["height"] = sol::property(&Map::get_height);
    map_type["tile_width"] = sol::property(&Map::get_tile_width);
    map_type["tile_height"] = sol::property(&Map::get_tile_height);
    map_type["filename"] = sol::property(&Map::get_filename);
    map_type["filename_stem"] = sol::property([](const Map& map) {
        return file_utilities::get_stem_component(map.get_filename());
    });
    map_type["name"] = sol::property(&Map::get_name);
    map_type["objects"] = sol::property([&](Map* map) {
        return sol::as_table(map->get_objects());
    });
    map_type["layer_count"] = sol::property(&Map::layer_count);
    map_type["object_count"] = sol::property(&Map::object_count);
    map_type["draw_outlines"] = sol::property(&Map::get_draw_outlines, &Map::set_draw_outlines);
    map_type["get_property"] = &Map::get_property;
    map_type["set_property"] = &Map::set_property;
    map_type["script_scheduler_paused"] = sol::property(&Map::is_script_scheduler_paused, &Map::set_script_scheduler_paused);
    map_type["get_object"] = sol::overload(
        (Map_Object* (Map::*)(int)) &Map::get_object,
        (Map_Object* (Map::*)(std::string)) &Map::get_object
    );
    map_type["add_new_object"] = &Map::add_new_object;
    map_type["delete_object"] = (void (Map::*)(Map_Object*)) &Map::delete_object;
    map_type["get_layer"] = sol::overload(
        &Map::get_layer_by_id,
        &Map::get_layer_by_name
    );
    map_type["get_image_layer"] = sol::overload(
        &Map::get_image_layer_by_id,
        &Map::get_image_layer_by_name
    );
    map_type["get_object_layer"] = sol::overload(
        &Map::get_object_layer_by_id,
        &Map::get_object_layer_by_name
    );
    map_type["run_script"] = &Map::run_script;
    map_type["run_script_file"] = &Map::run_script_file;
    map_type["run_function"] = &Map::run_function;
    map_type["passable"] = [&](Map& map, const Map_Object& object, int dir) {
        auto c = map.passable(object, static_cast<Direction>(dir));
        return c.passable();
    };
    map_type["colliding_object"] = [&](Map& map, const Map_Object& object) -> Map_Object* {
        auto c = map.passable(object, object.get_direction(),
            Collision_Check_Type::OBJECT);
        if (c.type == Collision_Type::OBJECT)
            return c.other_object;
        else
            return nullptr;
    };

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
        auto si = game->get_current_scripting_interface();
        return si->register_command<Move_Camera_Command>(camera, static_cast<Direction>(dir), pixels, speed);
    };
    camera_type["move_to"] = sol::overload(
        [&](Camera& camera, float x, float y, float speed) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Move_Camera_Command>(camera, x, y, speed);
        },
        [&](Camera& camera, xd::vec2 pos, float speed) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Move_Camera_Command>(camera, pos.x, pos.y, speed);
        },
        [&](Camera& camera, Map_Object& object, float speed) {
            auto pos = camera.get_centered_position(object);
            auto si = game->get_current_scripting_interface();
            return si->register_command<Move_Camera_Command>(camera, pos.x, pos.y, speed);
        }
    );
    camera_type["tint_screen"] = sol::overload(
        [&](Camera& camera, xd::vec4 color, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Tint_Command>(Tint_Target::SCREEN, *game, color, duration);
        },
        [&](Camera& camera, const std::string& hex_color, long duration) {
            auto si = game->get_current_scripting_interface();
            auto color = hex_to_color(hex_color);
            return si->register_command<Tint_Command>(Tint_Target::SCREEN, *game, color, duration);
        }
    );
    camera_type["tint_map"] = sol::overload(
        [&](Camera& camera, xd::vec4 color, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command<Tint_Command>(Tint_Target::MAP, *game, color, duration);
        },
        [&](Camera& camera, const std::string& hex_color, long duration) {
            auto si = game->get_current_scripting_interface();
            auto color = hex_to_color(hex_color);
            return si->register_command<Tint_Command>(Tint_Target::MAP, *game, color, duration);
        }
        );
    camera_type["zoom"] = [&](Camera& camera, float scale, long duration) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Zoom_Command>(*game, scale, duration);
    };
    camera_type["center_at"] = sol::overload(
        (void (Camera::*)(xd::vec2)) &Camera::center_at,
        (void (Camera::*)(const Map_Object&)) &Camera::center_at,
        [](Camera& camera, float x, float y) { camera.center_at(xd::vec2(x, y)); }
    );
    camera_type["track_object"] = [&](Camera& camera, Map_Object& object) { camera.set_object(&object); };
    camera_type["stop_tracking"] = [&](Camera& camera) { camera.set_object(nullptr); };
    camera_type["start_shaking"] = &Camera::start_shaking;
    camera_type["cease_shaking"] = &Camera::cease_shaking;
    camera_type["shake_screen"] = [&](Camera* camera, float strength, float speed, long duration) {
        auto si = game->get_current_scripting_interface();
        return si->register_command<Shake_Screen_Command>(*game, strength, speed, duration);
    };

    // Parsed token types
    lua.new_enum<Token_Type>("Token_Type",
        {
            {"text", Token_Type::TEXT},
            {"opening_tag", Token_Type::OPENING_TAG},
            {"closing_tag", Token_Type::CLOSING_TAG}
        }
    );

    // Parsed text token
    auto token_type = lua.new_usertype<Token>("Token");
    token_type["type"] = sol::readonly(&Token::type);
    token_type["tag"] = sol::readonly(&Token::tag);
    token_type["value"] = sol::readonly(&Token::value);
    token_type["unmatched"] = sol::readonly(&Token::unmatched);
    token_type["start_index"] = sol::readonly(&Token::start_index);
    token_type["end_index"] = sol::readonly(&Token::end_index);
    token_type["self_closing"] = sol::readonly(&Token::self_closing);

    // Text parser
    auto parser_type = lua.new_usertype<Text_Parser>("Text_Parser",
        sol::call_constructor, sol::constructors<Text_Parser()>());
    parser_type["parse"] = [&](Text_Parser& parser, const std::string& text, bool permissive) {
        return sol::as_table(parser.parse(text, permissive));
    };

    bind_canvas_types(lua, *game);
}
