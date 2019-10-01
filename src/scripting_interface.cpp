#include "../include/scripting_interface.hpp"
#include "../include/game.hpp"
#include "../include/clock.hpp"
#include "../include/camera.hpp"
#include "../include/map.hpp"
#include "../include/canvas.hpp"
#include "../include/collision_record.hpp"
#include "../include/image_layer.hpp"
#include "../include/map_object.hpp"
#include "../include/command_result.hpp"
#include "../include/commands.hpp"
#include "../include/utility.hpp"
#include "../include/configurations.hpp"
#include "../include/sprite_data.hpp"
#include "../include/direction_utilities.hpp"
#include "../include/save_file.hpp"
#include "../include/log.hpp"
#include "../include/xd/audio.hpp"
#include "../include/xd/lua.hpp"

Game* Scripting_Interface::game = nullptr;

Scripting_Interface::Scripting_Interface(Game& game) : scheduler(*game.get_lua_vm()) {
    if (!Scripting_Interface::game) {
        Scripting_Interface::game = &game;
        setup_scripts();
    }
}

void Scripting_Interface::update() {
    // Execute pending commands
    for (auto i = commands.begin(); i < commands.end();) {
        auto command = *i;
        command->execute();
        if (command->is_complete()) {
            i = commands.erase(i);
            continue;
        }
        ++i;
    }
    try {
        if (scheduler.pending_tasks() > 0)
            scheduler.run();
    } catch (const sol::error& e) {
        LOGGER_E << "Lua Error: " << e.what();
        throw;
    }
}

void Scripting_Interface::run_script(const std::string& script) {
    try {
        if (!script.empty())
            scheduler.start(script);
    } catch (const sol::error& e) {
        LOGGER_E << "Lua Error: " << e.what();
        throw;
    }
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
    // Object draw order
    vm.globals()["DRAW_BELOW"] = 0;
    vm.globals()["DRAW_NORMAL"] = 1;
    vm.globals()["DRAW_ABOVE"] = 2;
    // Text positioning
    vm.globals()["TEXT_POSITION_NONE"] = static_cast<int>(Text_Position_Type::NONE);
    vm.globals()["TEXT_POSITION_EXACT_X"] = static_cast<int>(Text_Position_Type::EXACT_X);
    vm.globals()["TEXT_POSITION_CENTERED_X"] = static_cast<int>(Text_Position_Type::CENTERED_X);
    vm.globals()["TEXT_POSITION_EXACT_Y"] = static_cast<int>(Text_Position_Type::EXACT_Y);
    vm.globals()["TEXT_POSITION_BOTTOM_Y"] = static_cast<int>(Text_Position_Type::BOTTOM_Y);
    vm.globals()["TEXT_POSITION_CAMERA"] = static_cast<int>(Text_Position_Type::CAMERA_RELATIVE);
    vm.globals()["TEXT_POSITION_ALWAYS_VISIBLE"] = static_cast<int>(Text_Position_Type::ALWAYS_VISIBLE);
}

std::unique_ptr<Command_Result> Scripting_Interface::register_command(std::shared_ptr<Command> command) {
    commands.push_back(command);
    return std::make_unique<Command_Result>(command);
}

std::unique_ptr<Choice_Result> Scripting_Interface::register_choice_command(std::shared_ptr<Command> command) {
    commands.push_back(command);
    return std::make_unique<Choice_Result>(command);
}

sol::state& Scripting_Interface::lua_state() {
    return game->get_lua_vm()->lua_state();
}

void Scripting_Interface::setup_scripts() {
    auto& vm = *game->get_lua_vm();

    if (Configurations::get<bool>("debug.seed-lua-rng")) {
        vm.lua_state().script("math.randomseed(os.time())");
    }
    auto result_wait = [](Command_Result* cmd) {
        auto& scheduler = game->get_current_scripting_interface()->scheduler;
        scheduler.yield(*cmd);
    };
    auto wait = [](Game& game, int duration) {
        int old_time = game.ticks();
        auto& scheduler = game.get_current_scripting_interface()->scheduler;
        scheduler.yield([&game, old_time, duration]() -> bool {
            return game.ticks() - old_time >= duration;
        });
    };
    auto wait_press = [](Game& game, const std::string& key) {
        auto& scheduler = game.get_current_scripting_interface()->scheduler;
        scheduler.yield([&game, key]() -> bool {
            return game.triggered(key);
        });
    };

    auto& lua = vm.lua_state();

    lua["wait"] = sol::yielding(sol::overload(
        [&](int duration) {
            wait(*game, duration);
        },
        [&](const std::string& key) {
            wait_press(*game, key);
        }
    ));

    lua["text_width"] = [&](const std::string& text) {
        return game->get_font()->get_width(text,
            xd::font_style(xd::vec4(1.0f, 1.0f, 1.0f, 1.0f), 8)
            .force_autohint(true));
    };

    // Logging
    lua["log_info"] = [](const std::string& message) {
        LOGGER_I << message;
    };
    lua["log_debug"] = [](const std::string& message) {
        LOGGER_D << message;
    };
    lua["log_warning"] = [](const std::string& message) {
        LOGGER_W << message;
    };
    lua["log_error"] = [](const std::string& message) {
        LOGGER_E << message;
    };

    // Bit operations
    lua["bitor"] = [](int a, int b) { return a | b; };
    lua["bitand"] = [](int a, int b) { return a & b; };
    lua["bitxor"] = [](int a, int b) { return a ^ b; };
    lua["bitnot"] = [](int a) { return ~a; };
    lua["rshift"] = [](int a, int b) { return a >> b; };
    lua["lshift"] = [](int a, int b) { return a << b; };

     // Direction utilities
    lua["opposite_direction"] = [](int dir) {
        return static_cast<int>(opposite_direction(static_cast<Direction>(dir)));
    };
    lua["direction_to_vector"] = [](int dir) {
        return direction_to_vector(static_cast<Direction>(dir));
    };
    lua["vector_to_direction"] = [](xd::vec2 vec) {
        return static_cast<int>(vector_to_direction(vec));
    };
    lua["direction_to_string"] = [](int dir) {
        return direction_to_string(static_cast<Direction>(dir));
    };
    lua["string_to_direction"] = [](const std::string& str) {
        return static_cast<int>(string_to_direction(str));
    };
    lua["facing_direction"] = sol::overload(
        [](xd::vec2 pos1, xd::vec2 pos2) {
            return static_cast<int>(facing_direction(pos1, pos2));
        },
        [](xd::vec2 pos1, xd::vec2 pos2, bool diagonal) {
            return static_cast<int>(facing_direction(pos1, pos2, diagonal));
        }
    );
    lua["is_diagonal"] = [](int dir) {
        return is_diagonal(static_cast<Direction>(dir));
    };

    // Returned from commands that allow yielding
    lua.new_usertype<Command_Result>("Command_Result",
        "is_complete", sol::overload(
            &Command_Result::operator(),
            (bool (Command_Result::*)(int) const) & Command_Result::is_complete),
        "execute", sol::overload(
        (void (Command_Result::*)()) & Command_Result::execute,
            (void (Command_Result::*)(int)) & Command_Result::execute),
        "wait", sol::yielding(result_wait),
        "stop", &Command_Result::stop
        );
    // Like Command_Result but stores the index of selected choice
    lua.new_usertype<Choice_Result>("Choice_Result",
        sol::base_classes, sol::bases<Command_Result>(),
        "selected", sol::property([](Choice_Result& cr) { return cr.choice_index() + 1; })
    );
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
            tile_only ? Collision_Check_Types::TILE : Collision_Check_Types::BOTH,
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
        return std::make_unique<Command_Result>(std::make_shared<Show_Pose_Command>(
            object, pose, state, direction));
    };

    // Options for displaying text
    lua.new_usertype<Text_Options>("Text_Options",
        sol::call_constructor, sol::constructors<Text_Options(), Text_Options(Map_Object*), Text_Options(xd::vec2)>(),
        "text", sol::readonly(&Text_Options::text),
        "choices", sol::readonly(&Text_Options::choices),
        "object", sol::readonly(&Text_Options::object),
        "position", sol::readonly(&Text_Options::position),
        "position_type", sol::readonly(&Text_Options::position_type),
        "duration", sol::readonly(&Text_Options::duration),
        "centered", sol::readonly(&Text_Options::centered),
        "show_dashes", sol::readonly(&Text_Options::show_dashes),
        "choice_indent", sol::readonly(&Text_Options::choice_indent),
        "translated", sol::readonly(&Text_Options::translated),
        "canvas_priority", sol::readonly(&Text_Options::canvas_priority),
        "fade_in_duration", sol::readonly(&Text_Options::fade_in_duration),
        "fade_out_duration", sol::readonly(&Text_Options::fade_out_duration),
        "set_text", &Text_Options::set_text,
        "set_object", &Text_Options::set_object,
        "set_position", &Text_Options::set_position,
        "set_duration", &Text_Options::set_duration,
        "set_centered", &Text_Options::set_centered,
        "set_show_dashes", &Text_Options::set_show_dashes,
        "set_translated", &Text_Options::set_translated,
        "set_choice_indent", &Text_Options::set_choice_indent,
        "set_canvas_priority", &Text_Options::set_canvas_priority,
        "set_fade_in_duration", &Text_Options::set_fade_in_duration,
        "set_fade_out_duration", &Text_Options::set_fade_out_duration,
        "set_choices", [&](Text_Options& options, const sol::table& table) -> Text_Options & {
            std::vector<std::string> choices;
            for (auto& kv : table) {
                choices.push_back(kv.second.as<std::string>());
            }
            return options.set_choices(choices);
        },
        "set_position_type", [&](Text_Options& options, int type) -> Text_Options & {
            return options.set_position_type(static_cast<Text_Position_Type>(type));
        }
    );

    // Show text
    lua["text"] = sol::overload(
        [&](const Text_Options& options) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Show_Text_Command>(*game, options));
        },
        [&](Map_Object& obj, const std::string& text) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Show_Text_Command>(
                *game, Text_Options{ &obj }.set_text(text)));
        },
        [&](xd::vec2& position, const std::string& text) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Show_Text_Command>(
                *game, Text_Options{ position }.set_text(text)));
        },
        [&](Map_Object& obj, const std::string& text, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Show_Text_Command>(
                *game, Text_Options{ &obj }.set_text(text).set_duration(duration)));
        },
        [&](xd::vec2& position, const std::string& text, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Show_Text_Command>(
                *game, Text_Options{ position }.set_text(text).set_duration(duration)));
        }
    );
    lua["centered_text"] = sol::overload(
        [&](float y, const std::string& text) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Show_Text_Command>(
                *game, Text_Options{ xd::vec2{0.0f, y} }.set_text(text).set_centered(true)
                ));
        },
        [&](float y, const std::string& text, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Show_Text_Command>(
                *game, Text_Options{ xd::vec2{0.0f, y} }.set_text(text).set_duration(duration).set_centered(true)
                ));
        }
    );

    // Show some text followed by a list of choices
    lua["choices"] = sol::overload(
        [&](const Text_Options& options) {
            auto si = game->get_current_scripting_interface();
            return si->register_choice_command(std::make_shared<Show_Text_Command>(*game, options));
        },
        [&](Map_Object& obj, const std::string& text, const sol::table& table) {
            std::vector<std::string> choices;
            for (auto& kv : table) {
                choices.push_back(kv.second.as<std::string>());
            }
            auto command = std::make_shared<Show_Text_Command>(
                *game, Text_Options{ &obj }.set_choices(choices).set_text(text));
            auto si = game->get_current_scripting_interface();
            return si->register_choice_command(command);
        },
        [&](xd::vec2& position, const std::string& text, const sol::table& table) {
            std::vector<std::string> choices;
            for (auto& kv : table) {
                choices.push_back(kv.second.as<std::string>());
            }
            auto command = std::make_shared<Show_Text_Command>(
                *game, Text_Options{ position }.set_choices(choices).set_text(text));
            auto si = game->get_current_scripting_interface();
            return si->register_choice_command(command);
        }
    );
    
    // 2D vector
    lua.new_usertype<xd::vec2>("Vec2",
        sol::call_constructor, sol::constructors<xd::vec2(), xd::vec2(const xd::vec2&), xd::vec2(float, float)>(),
        "x", &xd::vec2::x,
        "y", &xd::vec2::y,
        "length", [](xd::vec2& v) { return xd::length(v); },
        "normal", [](xd::vec2& v) { return xd::normalize(v); },
        sol::meta_function::addition, [](const xd::vec2& v1, const xd::vec2& v2) { return v1 + v2; },
        sol::meta_function::subtraction, [](const xd::vec2& v1, const xd::vec2& v2) { return v1 - v2; },
        sol::meta_function::multiplication, sol::overload(
            [](const xd::vec2& v1, float f) { return v1 * f; },
            [](float f, const xd::vec2& v1) { return f * v1; }
        )
    );
    // 3D vector
    lua.new_usertype<xd::vec3>("Vec3",
        sol::call_constructor, sol::constructors<xd::vec3(), xd::vec3(const xd::vec3&), xd::vec3(float, float, float)>(),
        "x", &xd::vec3::x,
        "y", &xd::vec3::y,
        "z", &xd::vec3::z,
        "length", [](xd::vec3& v) { return xd::length(v); },
        "normal", [](xd::vec3& v) { return xd::normalize(v); },
        sol::meta_function::addition, [](const xd::vec3& v1, const xd::vec3& v2) { return v1 + v2; },
        sol::meta_function::subtraction, [](const xd::vec3& v1, const xd::vec3& v2) { return v1 - v2; },
        sol::meta_function::multiplication, sol::overload(
            [](const xd::vec3& v1, float f) { return v1 * f; },
            [](float f, const xd::vec3& v1) { return f * v1; }
        )
    );
    // 4D vector (or color)
    lua.new_usertype<xd::vec4>("Vec4",
        sol::call_constructor, sol::constructors<xd::vec4(), xd::vec4(const xd::vec4&), xd::vec4(float, float, float, float)>(),
        "x", &xd::vec4::x,
        "y", &xd::vec4::y,
        "z", &xd::vec4::z,
        "w", &xd::vec4::w,
        "r", &xd::vec4::r,
        "g", &xd::vec4::g,
        "b", &xd::vec4::b,
        "a", &xd::vec4::a,
        "length", [](xd::vec4& v) { return xd::length(v); },
        "normal", [](xd::vec4& v) { return xd::normalize(v); },
        sol::meta_function::addition, [](const xd::vec4& v1, const xd::vec4& v2) { return v1 + v2; },
        sol::meta_function::subtraction, [](const xd::vec4& v1, const xd::vec4& v2) { return v1 - v2; },
        sol::meta_function::multiplication, sol::overload(
            [](const xd::vec4& v1, float f) { return v1 * f; },
            [](float f, const xd::vec4& v1) { return f * v1; }
        )
    );
    // Aliases for creating a color
    lua["Color"] = sol::overload(
        [](float r, float g, float b, float a) { return std::make_unique<xd::vec4>(r, g, b, a); },
        [](float r, float g, float b) { return std::make_unique<xd::vec4>(r, g, b, 1.0f); },
        [](std::string name) {
            if (name == "clear") {
                auto clear = hex_to_color(
                    Configurations::get<std::string>("startup.clear-color"));
                return std::make_unique<xd::vec4>(clear);
            }
            else if (name == "none")
                return std::make_unique<xd::vec4>();
            else if (name == "black")
                return std::make_unique<xd::vec4>(0.0f, 0.0f, 0.0f, 1.0f);
            else if (name == "red")
                return std::make_unique<xd::vec4>(1.0f, 0.0f, 0.0f, 1.0f);
            else if (name == "green")
                return std::make_unique<xd::vec4>(0.0f, 1.0f, 0.0f, 1.0f);
            else if (name == "blue")
                return std::make_unique<xd::vec4>(0.0f, 0.0f, 1.0f, 1.0f);
            else if (name == "yellow")
                return std::make_unique<xd::vec4>(1.0f, 1.0f, 0.0f, 1.0f);
            else if (name == "white")
                return std::make_unique<xd::vec4>(1.0f, 1.0f, 1.0f, 1.0f);
            else {
                try {
                    auto color = hex_to_color(name);
                    return std::make_unique<xd::vec4>(color);
                }
                catch (std::runtime_error&) {}
            }
            return std::make_unique<xd::vec4>();
        }
    );
    // Rectangle
    lua.new_usertype<xd::rect>("Rect",
        sol::call_constructor, sol::constructors<
        xd::rect(),
        xd::rect(const xd::rect&),
        xd::rect(float, float, float, float),
        xd::rect(const xd::vec2&, const xd::vec2&),
        xd::rect(const xd::vec2&, float, float),
        xd::rect(const xd::vec4&)>(),
        "x", &xd::rect::x,
        "y", &xd::rect::y,
        "w", &xd::rect::w,
        "h", &xd::rect::h,
        "intersects", &xd::rect::intersects
    );

    // Sprite holders: map object, image layer, canvas, etc.
    lua.new_usertype<Sprite_Holder>("Sprite_Holder",
        "reset", &Sprite_Holder::reset,
        "set_sprite", sol::overload(
            [&](Sprite_Holder* holder, const std::string& filename) {
                holder->set_sprite(*game, filename);
            },
            [&](Sprite_Holder* holder, const std::string& filename, const std::string& pose) {
                holder->set_sprite(*game, filename, pose);
            }
        ),
        "show_pose", sol::overload(
            [&](Sprite_Holder* obj, const std::string& pose_name) {
                return std::make_unique<Command_Result>(std::make_shared<Show_Pose_Command>(obj, pose_name));
            },
            [&](Sprite_Holder* obj, const std::string& pose_name, const std::string& state) {
                return std::make_unique<Command_Result>(std::make_shared<Show_Pose_Command>(obj, pose_name, state));
            },
            [&](Sprite_Holder* obj, const std::string& pose_name, const std::string& state, Direction dir) {
                return std::make_unique<Command_Result>(std::make_shared<Show_Pose_Command>(obj, pose_name, state, dir));
            }
        )
    );
    // Map object
    lua.new_usertype<Map_Object>("Map_Object",
        sol::base_classes, sol::bases<Sprite_Holder>(),
        "id", sol::property(&Map_Object::get_id),
        "name", sol::property(&Map_Object::get_name, &Map_Object::set_name),
        "type", sol::property(&Map_Object::get_type, &Map_Object::set_type),
        "position", sol::property(&Map_Object::get_position, &Map_Object::set_position),
        "x", sol::property(&Map_Object::get_x, &Map_Object::set_x),
        "y", sol::property(&Map_Object::get_y, &Map_Object::set_y),
        "color", sol::property(&Map_Object::get_color, &Map_Object::set_color),
        "opacity", sol::property(&Map_Object::get_opacity, &Map_Object::set_opacity),
        "disabled", sol::property(&Map_Object::is_disabled, &Map_Object::set_disabled),
        "stopped", sol::property(&Map_Object::is_stopped, &Map_Object::set_stopped),
        "frozen", sol::property(&Map_Object::is_frozen, &Map_Object::set_frozen),
        "passthrough", sol::property(&Map_Object::is_passthrough, &Map_Object::set_passthrough),
        "pose_name", sol::property(&Map_Object::get_pose_name),
        "state", sol::property(&Map_Object::get_state, &Map_Object::update_state),
        "walk_state", sol::property(&Map_Object::get_walk_state, &Map_Object::set_walk_state),
        "face_state", sol::property(&Map_Object::get_face_state, &Map_Object::set_face_state),
        "visible", sol::property(&Map_Object::is_visible, &Map_Object::set_visible),
        "script", sol::property(&Map_Object::get_trigger_script_source, &Map_Object::set_trigger_script_source),
        "triggered_object", sol::property(&Map_Object::get_triggered_object, &Map_Object::set_triggered_object),
        "collision_object", sol::property(&Map_Object::get_collision_object, &Map_Object::set_collision_object),
        "collision_area", sol::property(&Map_Object::get_collision_area, &Map_Object::set_collision_area),
        "outlined", sol::property(&Map_Object::is_outlined),
        "draw_order", sol::property(&Map_Object::get_draw_order, &Map_Object::set_draw_order),
        "real_position", sol::property(&Map_Object::get_real_position),
        "bounding_box", sol::property(&Map_Object::get_bounding_box),
        "speed", sol::property(&Map_Object::get_speed, &Map_Object::set_speed),
        "angle", sol::property(&Map_Object::get_angle, &Map_Object::set_angle),
        "direction", sol::property(
            [](Map_Object* obj) {
                return static_cast<int>(obj->get_direction());
            },
            [](Map_Object* obj, int dir) {
                obj->set_direction(static_cast<Direction>(dir));
            }
        ),
        "magnification", sol::property(
            [](Map_Object* obj) -> xd::vec2 {
                auto sprite = obj->get_sprite();
                if (sprite)
                    return sprite->get_frame().magnification;
                else
                    return xd::vec2(1.0f, 1.0f);
            }
        ),
        "get_property", &Map_Object::get_property,
        "set_property", &Map_Object::set_property,
        "move_to", sol::overload(
            [&](Map_Object* obj, float x, float y) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Object_To_Command>(
                        *game->get_map(), *obj, x, y
                        )
                );
            },
            [&](Map_Object* obj, float x, float y, bool keep_trying) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Object_To_Command>(
                        *game->get_map(), *obj, x, y,
                        Collision_Check_Types::BOTH, keep_trying
                        )
                );
            }
        ),
        "move", sol::overload(
            [&](Map_Object* obj, int dir, float pixels, bool skip, bool change_facing) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Object_Command>(
                        *obj, static_cast<Direction>(dir),
                        pixels, skip, change_facing
                        )
                );
            },
            [&](Map_Object* obj, int dir, float pixels, bool skip) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Object_Command>(
                        *obj, static_cast<Direction>(dir), pixels, skip, true
                        )
                );
            },
            [&](Map_Object* obj, int dir, float pixels) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Object_Command>(
                        *obj, static_cast<Direction>(dir), pixels, true, true
                        )
                );
            }
        ),
        "face", sol::overload(
            (void (Map_Object::*)(xd::vec2))& Map_Object::face,
            (void (Map_Object::*)(float, float))& Map_Object::face,
            (void (Map_Object::*)(const Map_Object&))& Map_Object::face,
            [&](Map_Object* obj, int dir) {
                obj->face(static_cast<Direction>(dir));
            }
        ),
        "run_script", &Map_Object::run_trigger_script
    );

    // Sound effect
    lua.new_usertype<xd::sound>("Sound",
        sol::call_constructor, sol::factories(
            [&](const std::string& filename) {
                return std::make_unique<xd::sound>(*game->get_audio(), filename);
            }
        ),
        "playing", sol::property(&xd::sound::playing),
        "paused", sol::property(&xd::sound::paused),
        "stopped", sol::property(&xd::sound::stopped),
        "offset", sol::property(&xd::sound::get_offset, &xd::sound::set_offset),
        "volume", sol::property(&xd::sound::get_volume, &xd::sound::set_volume),
        "pitch", sol::property(&xd::sound::get_pitch, &xd::sound::set_pitch),
        "looping", sol::property(&xd::sound::get_looping, &xd::sound::set_looping),
        "filename", sol::property(&xd::sound::get_filename),
        "play", &xd::sound::play,
        "pause", &xd::sound::pause,
        "stop", &xd::sound::stop,
        "set_loop_points", &xd::sound::set_loop_points
    );
    
    // Background music
    lua.new_usertype<xd::music>("Music",
        "playing", sol::property(&xd::music::playing),
        "paused", sol::property(&xd::music::paused),
        "stopped", sol::property(&xd::music::stopped),
        "offset", sol::property(&xd::music::get_offset, &xd::music::set_offset),
        "volume", sol::property(&xd::music::get_volume, &xd::music::set_volume),
        "pitch", sol::property(&xd::music::get_pitch, &xd::music::set_pitch),
        "looping", sol::property(&xd::music::get_looping, &xd::music::set_looping),
        "filename", sol::property(&xd::music::get_filename),
        "play", &xd::music::play,
        "pause", &xd::music::pause,
        "stop", &xd::music::stop,
        "set_loop_points", &xd::music::set_loop_points,
        "fade", [&](xd::music* music, float volume, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Fade_Music_Command>(
                    *game, volume, duration
                    )
            );
        }
    );

    // Map object
    lua.new_usertype<Game>("Game",
        "width", sol::property(&Game::width),
        "height", sol::property(&Game::height),
        "game_width", sol::property([](Game& game) { return game.game_width(); }),
        "game_height", sol::property([](Game& game) { return game.game_height(); }),
        "magnification", sol::property(&Game::get_magnification, &Game::set_magnification),
        "ticks", sol::property(&Game::ticks),
        "fps", sol::property(&Game::fps),
        "frame_count", sol::property(&Game::frame_count),
        "stopped", sol::property(&Game::stopped),
        "seconds", sol::property(&Game::seconds),
        "sizes", sol::property(
            [&](Game& game) {
                sol::table objects(vm.lua_state(), sol::create);
                auto sizes = game.get_sizes();
                for (auto i = 0u; i < sizes.size(); ++i) {
                    objects[i + 1] = sizes[i];
                }
                return objects;
            }
        ),
        "playing_music", sol::property([](Game* game) { return game->playing_music().get(); }),
        "set_size", & Game::set_size,
        "exit", &Game::exit,
        "pressed", [](Game* game, const std::string& key) { return game->pressed(key); },
        "triggered", [](Game* game, const std::string& key) { return game->triggered(key); },
        "triggered_once", [](Game* game, const std::string& key) { return game->triggered_once(key); },
        "run_script", & Game::run_script,
        "stop_time", [](Game* game) { game->get_clock()->stop_time(); },
        "resume_time", [](Game* game) { game->get_clock()->resume_time(); },
        "load_map", sol::overload(
            [](Game* game, const std::string& filename) {
                game->set_next_map(filename, Direction::NONE);
            },
            [](Game* game, const std::string& filename, int dir) {
                game->set_next_map(filename, static_cast<Direction>(dir));
            },
            [](Game* game, const std::string& filename, float x, float y, int dir) {
                game->set_next_map(filename, x, y, static_cast<Direction>(dir));
            }
        ),
        "get_config", [](Game*, const std::string& key) { return Configurations::get_string(key); },
        "set_bool_config", [](Game*, const std::string& key, bool value) {
            Configurations::set(key, value);
        },
        "set_float_config", [](Game*, const std::string& key, float value) {
            Configurations::set(key, value);
        },
        "set_int_config", [](Game*, const std::string& key, int value) {
            Configurations::set(key, value);
        },
        "set_unsigned_config", [](Game*, const std::string& key, unsigned int value) {
            Configurations::set(key, value);
        },
        "set_string_config", [](Game*, const std::string& key, const std::string& value) {
            Configurations::set(key, value);
        },
        "save", [&](Game* game, const std::string& filename, sol::table obj) {
            Save_File file(vm.lua_state(), obj);
            game->save(filename, file);
            return file.is_valid();
        },
        "load", [&](Game* game, const std::string& filename) {
            auto file = game->load(filename);
            if (file->is_valid())
                return file->lua_data();
            else
                return sol::object(sol::nil);
        },
        "load_music", [&](Game* game, const std::string& filename) {
            game->load_music(filename);
            return game->playing_music();
        },
        "play_music", [&](Game* game, const std::string& filename) {
            game->load_music(filename);
            game->playing_music()->play();
        }
    );

    // Map layer
    lua.new_usertype<Layer>("Layer",
        "name", sol::readonly(&Layer::name),
        "visible", &Layer::visible,
        "opacity", &Layer::opacity,
        "get_property", &Layer::get_property,
        "set_property", &Layer::set_property,
        "update_opacity", [&](Layer* layer, float opacity, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Update_Layer_Command>(
                    *game, *layer, opacity, duration
                    )
            );
        }
    );
    // Image layer
    lua.new_usertype<Image_Layer>("Image_Layer",
        sol::base_classes, sol::bases<Layer, Sprite_Holder>(),
        "velocity", &Image_Layer::velocity
    );

    // Current map / scene
    lua.new_usertype<Map>("Map",
        "width", sol::property(&Map::get_width),
        "height", sol::property(&Map::get_height),
        "tile_width", sol::property(&Map::get_tile_width),
        "tile_height", sol::property(&Map::get_tile_height),
        "filename", sol::property(&Map::get_filename),
        "name", sol::property(&Map::get_name),
        "get_property", &Map::get_property,
        "set_property", &Map::set_property,
        "object_count", &Map::object_count,
        "get_object", sol::overload(
            (Map_Object* (Map::*)(int))& Map::get_object,
            (Map_Object* (Map::*)(std::string))& Map::get_object
        ),
        "add_new_object", &Map::add_new_object,
        "add_object", [](Map& map, Map_Object& object, int layer_index) {
            return map.add_object(&object, layer_index);
        },
        "delete_object", (void (Map::*)(Map_Object*))& Map::delete_object,
        "layer_count", & Map::layer_count,
        "get_layer", sol::overload(
            (Layer* (Map::*)(int))& Map::get_layer,
            (Layer* (Map::*)(std::string))& Map::get_layer
        ),
        "get_objects", [&](Map* map) {
            sol::table objects(vm.lua_state(), sol::create);
            int i = 1;
            auto& map_objects = map->get_objects();
            for (auto pair : map_objects) {
                objects[i++] = pair.second.get();
            }
            return objects;
        },
        "run_script", & Map::run_script,
        "passable", [&](Map& map, const Map_Object& object, int dir) {
            auto c = map.passable(object, static_cast<Direction>(dir));
            return c.passable();
        },
        "colliding_object", [&](Map& map, const Map_Object& object) -> Map_Object* {
            auto c = map.passable(object, object.get_direction(),
                Collision_Check_Types::OBJECT);
            if (c.type == Collision_Types::OBJECT)
                return c.other_object;
            else
                return nullptr;
        }
    );

    // Camera tracking and effects
    lua.new_usertype<Camera>("Camera",
        "tint_color", sol::property(&Camera::get_tint_color, &Camera::set_tint_color),
        "position", sol::property(&Camera::get_position, &Camera::set_position),
        "move", [&](Camera& camera, int dir, float pixels, float speed) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Move_Camera_Command>(camera, static_cast<Direction>(dir), pixels, speed)
            );
        },
        "move_to", sol::overload(
            [&](Camera& camera, float x, float y, float speed) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Camera_Command>(camera, x, y, speed)
                );
            },
            [&](Camera& camera, Map_Object& object, float speed) {
                xd::vec2 position = object.get_position();
                auto sprite = object.get_sprite();
                float width = sprite ? sprite->get_size().x : 0.0f;
                float height = sprite ? sprite->get_size().y : 0.0f;
                float x = position.x + width / 2 - game->game_width() / 2;
                float y = position.y + height / 2 - game->game_height() / 2;
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Camera_Command>(camera, x, y, speed)
                );
            }
        ),
        "tint_screen", sol::overload(
            [&](Camera& camera, xd::vec4 color, long duration) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Tint_Screen_Command>(*game, color, duration)
                );
            },
            [&](Camera& camera, const std::string& hex_color, long duration) {
                auto si = game->get_current_scripting_interface();
                auto color = hex_to_color(hex_color);
                return si->register_command(
                    std::make_shared<Tint_Screen_Command>(*game, color, duration)
                );
            }
        ),
        "zoom", [&](Camera& camera, float scale, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(std::make_shared<Zoom_Command>(*game, scale, duration));
        },
        "center_at", sol::overload(
            (void (Camera::*)(xd::vec2))& Camera::center_at,
            (void (Camera::*)(const Map_Object&))& Camera::center_at,
            [](Camera& camera, float x, float y) { camera.center_at(xd::vec2(x, y)); }
        ),
        "track_object", [&](Camera& camera, Map_Object& object) { camera.set_object(&object); },
        "start_shaking", & Camera::start_shaking,
        "cease_shaking", & Camera::cease_shaking,
        "shake_screen", [&](Camera* camera, float strength, float speed, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Shake_Screen_Command>(
                    *game, strength, speed, duration
                    )
            );
        }
    );

    // Parsed text token
    lua.new_usertype<Token>("Token",
        "type", sol::readonly(&Token::type),
        "tag", sol::readonly(&Token::tag),
        "value", sol::readonly(&Token::value),
        "unmatched", sol::readonly(&Token::unmatched),
        "start_index", sol::readonly(&Token::start_index),
        "end_index", sol::readonly(&Token::end_index)
    );

    // Text parser
    lua.new_usertype<Text_Parser>("Text_Parser",
        sol::call_constructor, sol::constructors<Text_Parser()>(),
        "parse", [&](Text_Parser& parser, const std::string& text, bool permissive) {
            sol::table objects(vm.lua_state(), sol::create);
            auto tokens = parser.parse(text, permissive);
            for (size_t i = 1; i <= tokens.size(); ++i) {
                objects[i] = tokens[i - 1];
            }
            return objects;
        }
    );

    // A drawing canvas
    lua.new_usertype<Canvas>("Canvas",
        sol::call_constructor, sol::factories(
            // Canvas constructor (with position)
            [&](const std::string& filename, float x, float y) {
                std::shared_ptr<Canvas> canvas;
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    canvas = std::make_shared<Canvas>(*game, filename, "", xd::vec2(x, y));
                else
                    canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y));
                game->add_canvas(canvas);
                return canvas;
            },
            // Canvas constructor (with transparent color as vec4)
            [&](const std::string& filename, float x, float y, const xd::vec4& trans) {
                auto canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y), trans);
                game->add_canvas(canvas);
                return canvas;
            },
            // Canvas constructor (with hex trans color or sprite with pose name)
            [&](const std::string& filename, float x, float y, const std::string& trans_or_pose) {
                std::shared_ptr<Canvas> canvas;
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    canvas = std::make_shared<Canvas>(*game, filename, trans_or_pose, xd::vec2(x, y));
                else
                    canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y), hex_to_color(trans_or_pose));
                game->add_canvas(canvas);
                return canvas;
            },
            // Canvas constructor (with text and position)
            [&](float x, float y, const std::string& text) {
                auto canvas = std::make_shared<Canvas>(*game, xd::vec2(x, y), text);
                game->add_canvas(canvas);
                return canvas;
            }
        ),
        sol::base_classes, sol::bases<Sprite_Holder>(),
        "name", sol::property(&Canvas::get_name, &Canvas::set_name),
        "priority", sol::property(&Canvas::get_priority, &Canvas::set_priority),
        "position", sol::property(&Canvas::get_position, &Canvas::set_position),
        "x", sol::property(&Canvas::get_x, &Canvas::set_x),
        "y", sol::property(&Canvas::get_y, &Canvas::set_y),
        "pose_name", sol::property(&Canvas::get_pose_name),
        "pose_state", sol::property(&Canvas::get_pose_state),
        "pose_direction", sol::property(&Canvas::get_pose_direction),
        "origin", sol::property(&Canvas::get_origin, &Canvas::set_origin),
        "magnification", sol::property(&Canvas::get_magnification, &Canvas::set_magnification),
        "scissor_box", sol::property(&Canvas::get_scissor_box, &Canvas::set_scissor_box),
        "angle", sol::property(&Canvas::get_angle, &Canvas::set_angle),
        "opacity", sol::property(&Canvas::get_opacity, &Canvas::set_opacity),
        "color", sol::property(&Canvas::get_color, &Canvas::set_color),
        "filename", sol::property(&Canvas::get_filename),
        "text", sol::property(&Canvas::get_text, &Canvas::set_text),
        "width", sol::property(&Canvas::get_width),
        "height", sol::property(&Canvas::get_height),
        "font_size", sol::property(&Canvas::get_font_size, &Canvas::set_font_size),
        "text_color", sol::property(&Canvas::get_text_color, &Canvas::set_text_color),
        "line_height", sol::property(&Canvas::get_line_height, &Canvas::set_line_height),
        "text_outline_width", sol::property(&Canvas::get_text_outline_width, &Canvas::set_text_outline_width),
        "text_outline_color", sol::property(&Canvas::get_text_outline_color, &Canvas::set_text_outline_color),
        "text_shadow_offset", sol::property(&Canvas::get_text_shadow_offset, &Canvas::set_text_shadow_offset),
        "text_shadow_color", sol::property(&Canvas::get_text_shadow_color, &Canvas::set_text_shadow_color),
        "text_type", sol::property(&Canvas::get_text_type, &Canvas::set_text_type),
        "child_count", sol::property(&Canvas::get_child_count),
        "permissive_tag_parsing", sol::property(&Canvas::get_permissive_tag_parsing, &Canvas::set_permissive_tag_parsing),
        "has_image_outline", sol::property(&Canvas::has_image_outline, &Canvas::set_image_outline),
        "image_outline_color", sol::property(&Canvas::get_image_outline_color, &Canvas::set_image_outline_color),
        "reset_text_outline", &Canvas::reset_text_outline,
        "reset_text_shadow", &Canvas::reset_text_shadow,
        "reset_text_type", &Canvas::reset_text_type,
        "set_font", &Canvas::set_font,
        "link_font", &Canvas::link_font,
        "remove_child", &Canvas::remove_child,
        "get_child", sol::overload(
            (Canvas* (Canvas::*)(std::size_t))& Canvas::get_child,
            (Canvas* (Canvas::*)(const std::string&))& Canvas::get_child
        ),
        "show", [](Canvas& canvas) {
            canvas.set_visible(true);
        },
        "hide", [](Canvas& canvas) {
            canvas.set_visible(false);
        },
        "set_image", sol::overload(
            [](Canvas& canvas, const std::string& filename) { canvas.set_image(filename); },
            [](Canvas& canvas, const std::string& filename, xd::vec4 ck) { canvas.set_image(filename, ck); }
        ),
        "update", [&](Canvas& canvas, float x, float y, float mag_x,
                float mag_y, float angle, float opacity, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Update_Canvas_Command>(
                    *game, canvas, duration, xd::vec2(x, y),
                    xd::vec2(mag_x, mag_y), angle, opacity
                    )
            );
        },
        "move", [&](Canvas& canvas, float x, float y, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Update_Canvas_Command>(
                    *game, canvas, duration,
                    xd::vec2(x, y), canvas.get_magnification(),
                    canvas.get_angle(), canvas.get_opacity()
                    )
            );
        },
        "resize", [&](Canvas& canvas, float mag_x, float mag_y, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Update_Canvas_Command>(
                    *game, canvas, duration,
                    canvas.get_position(), xd::vec2(mag_x, mag_y),
                    canvas.get_angle(), canvas.get_opacity()
                    )
            );
        },
        "update_opacity", [&](Canvas& canvas, float opacity, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Update_Canvas_Command>(
                    *game, canvas, duration, canvas.get_position(),
                    canvas.get_magnification(), canvas.get_angle(), opacity
                    )
            );
        },
        "rotate", [&](Canvas& canvas, float angle, long duration) {
            auto si = game->get_current_scripting_interface();
            return si->register_command(
                std::make_shared<Update_Canvas_Command>(
                    *game, canvas, duration, canvas.get_position(),
                    canvas.get_magnification(), angle, canvas.get_opacity()
                    )
            );
        },
        "add_child_image", sol::overload(
            // with position
            [&](Canvas& parent, const std::string& name, const std::string& filename, float x, float y) -> Canvas* {
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    return parent.add_child(name, *game, filename, "", xd::vec2(x, y));
                else
                    return parent.add_child(name, filename, xd::vec2(x, y));
            },
            // with hex trans color or sprite with pose name
            [&](Canvas& parent, const std::string& name, const std::string& filename, float x, float y, const std::string& trans_or_pose) -> Canvas* {
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    return parent.add_child(name, *game, filename, trans_or_pose, xd::vec2(x, y));
                else
                    return parent.add_child(name, filename, xd::vec2(x, y), hex_to_color(trans_or_pose));
            },
            // with transparent color as vec4
            [&](Canvas& parent, const std::string& name, const std::string& filename, float x, float y, const xd::vec4& trans) -> Canvas* {
                return parent.add_child(name, filename, xd::vec2(x, y), trans);
            },
            // with text and position
            [&](Canvas& parent, const std::string& name, float x, float y, const std::string& text) -> Canvas* {
                return parent.add_child(name, *game, xd::vec2(x, y), text, true);
            }
        )
    );
}
