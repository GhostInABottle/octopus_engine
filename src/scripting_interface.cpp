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
#include "../include/vendor/lutf8lib.hpp"
#include <luabind/adopt_policy.hpp>
#include <luabind/tag_function.hpp>
#include <luabind/std_shared_ptr_converter.hpp>

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
    } catch (const luabind::error& e) {
        std::string err(lua_tostring(e.state(), -1));
        LOGGER_E << "Lua Error: " << err;
        throw;
    }
}

void Scripting_Interface::run_script(const std::string& script) {
    try {
        if (!script.empty())
            scheduler.start(game->get_lua_vm()->load(script));
    } catch (const luabind::error& e) {
        std::string err(lua_tostring(e.state(), -1));
        LOGGER_E << "Lua Error: " << err;
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

Command_Result* Scripting_Interface::register_command(std::shared_ptr<Command> command) {
    commands.push_back(command);
    return new Command_Result(command);
}

Choice_Result* Scripting_Interface::register_choice_command(std::shared_ptr<Command> command) {
    commands.push_back(command);
    return new Choice_Result(command);
}

lua_State* Scripting_Interface::lua_state() {
    return game->get_lua_vm()->lua_state();
}

void Scripting_Interface::setup_scripts() {
    using namespace luabind;
    auto& vm = *game->get_lua_vm();
    vm.load_library();
    luaL_openlibs(vm.lua_state());
    luaopen_utf8(vm.lua_state());
    if (Configurations::get<bool>("debug.seed-lua-rng")) {
        vm.exec("math.randomseed(os.time())");
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

    module(vm.lua_state())
    [
        // Returned from commands that allow yielding
        class_<Command_Result>("Command_Result")
            .def("is_complete", &Command_Result::operator())
            .def("is_complete", (bool (Command_Result::*)(int) const) &Command_Result::is_complete)
            .def("execute", (void (Command_Result::*)()) &Command_Result::execute)
            .def("execute",  (void (Command_Result::*)(int)) &Command_Result::execute)
            .def("wait", tag_function<void (Command_Result*)>(result_wait), yield)
            .def("stop", &Command_Result::stop),
        def("wait", tag_function<void (int)>([&](int duration) {
            wait(*game, duration);
        }), yield),
        def("wait", tag_function<void (const std::string&)>([&](const std::string& key) {
            wait_press(*game, key);
        }), yield),
        def("text_width", tag_function<float (const std::string&)>(
            [&](const std::string& text) {
            return game->get_font()->get_width(text,
                xd::font_style(xd::vec4(1.0f, 1.0f, 1.0f, 1.0f), 8)
                .force_autohint(true));
            }
        )),
        def("log_info", tag_function<void(const std::string&)>(
            [](const std::string& message) {
                LOGGER_I << message;
            }
        )),
        def("log_debug", tag_function<void(const std::string&)>(
            [](const std::string& message) {
                LOGGER_D << message;
            }
        )),
        def("log_warning", tag_function<void(const std::string&)>(
            [](const std::string& message) {
                LOGGER_W << message;
            }
        )),
        def("log_error", tag_function<void(const std::string&)>(
            [](const std::string& message) {
                LOGGER_E << message;
            }
        )),
        def("bitor", tag_function<int(int, int)>([](int a, int b) { return a | b; })),
        def("bitand", tag_function<int(int, int)>([](int a, int b) { return a & b; })),
        def("bitxor", tag_function<int(int, int)>([](int a, int b) { return a ^ b; })),
        def("bitnot", tag_function<int(int)>([](int a) { return ~a; })),
        def("rshift", tag_function<int(int, int)>([](int a, int b) { return a >> b; })),
        def("lshift", tag_function<int(int, int)>([](int a, int b) { return a << b; })),
        // A generic command for waiting (used in NPC scheduling)
        def("Wait_Command", tag_function<Command_Result* (int, int)>(
            [&](int duration, int start_time) {
                return new Command_Result(std::make_shared<Wait_Command>(
                    *game,
                    duration,
                    start_time));
            }
        ), adopt(result)),
        // A command for moving an object (used in NPC scheduling)
        def("Move_To_Command", tag_function<Command_Result* (Map_Object*, float, float, bool, bool)>(
            [&](Map_Object* obj, float x, float y, bool keep_trying, bool tile_only) {
                return new Command_Result(std::make_shared<Move_Object_To_Command>(
                    *game->get_map(),
                    *obj,
                    x,
                    y,
                    tile_only ? Collision_Check_Types::TILE : Collision_Check_Types::BOTH,
                    keep_trying));
            }
        ), adopt(result)),
        // A command for showing text (used in NPC scheduling)
        def("Text_Command", tag_function<Command_Result* (Text_Options, long)>(
            [&](Text_Options options, long start_time) {
                auto command = std::make_shared<Show_Text_Command>(*game, options);

                if (start_time >= 0) {
                    command->set_start_time(start_time);
                }
                return new Command_Result(command);
            }
        ), adopt(result)),
        def("Text_Command", tag_function<Command_Result* (Map_Object*, const std::string&, long, long)>(
            [&](Map_Object* object, const std::string& text, long duration, long start_time) {
                Text_Options options(object);
                options.set_text(text)
                    .set_duration(duration)
                    .set_position_type(Text_Position_Type::CENTERED_X | Text_Position_Type::BOTTOM_Y);

                auto command = std::make_shared<Show_Text_Command>(*game, options);

                if (start_time >= 0) {
                    command->set_start_time(start_time);
                }
                return new Command_Result(command);
            }
        ), adopt(result)),
        // A command to show an object's pose (used in NPC scheduling)
        def("Pose_Command", tag_function<Command_Result* (Map_Object*, const std::string&,
                const std::string&, Direction)>(
            [&](Map_Object* object, const std::string& pose,
                    const std::string& state, Direction direction) {
                return new Command_Result(std::make_shared<Show_Pose_Command>(
                    object, pose, state, direction));
            }
        ), adopt(result)),
        // 2D vector
        class_<xd::vec2>("Vec2")
            .def(constructor<>())
            .def(constructor<const xd::vec2&>())
            .def(constructor<float, float>())
            .def(self + other<xd::vec2>())
            .def(self - other<xd::vec2>())
            .def(self * float())
            .def(float() * self)
            .def("length", tag_function<float (xd::vec2&)>([&](xd::vec2& v) { return xd::length(v); }))
            .def("normal", tag_function<xd::vec2 (xd::vec2&)>([&](xd::vec2& v) { return xd::normalize(v); }))
            .def_readwrite("x", &xd::vec2::x)
            .def_readwrite("y", &xd::vec2::y),
        // 3D vector
        class_<xd::vec3>("Vec3")
            .def(constructor<>())
            .def(constructor<const xd::vec3&>())
            .def(constructor<float, float, float>())
            .def(self + other<xd::vec3>())
            .def(self - other<xd::vec3>())
            .def(self * float())
            .def(float() * self)
            .def("length", tag_function<float (xd::vec3&)>([&](xd::vec3& v) { return xd::length(v); }))
            .def("normal", tag_function<xd::vec3 (xd::vec3&)>([&](xd::vec3& v) { return xd::normalize(v); }))
            .def_readwrite("x", &xd::vec3::x)
            .def_readwrite("y", &xd::vec3::y)
            .def_readwrite("z", &xd::vec3::z),
        // 4D vector (or color)
        class_<xd::vec4>("Vec4")
            .def(constructor<>())
            .def(constructor<const xd::vec4&>())
            .def(constructor<float, float, float, float>())
            .def(self + other<xd::vec4>())
            .def(self - other<xd::vec4>())
            .def(self * float())
            .def(float() * self)
            .def("length", tag_function<float (xd::vec4&)>([&](xd::vec4& v) { return xd::length(v); }))
            .def("normal", tag_function<xd::vec4 (xd::vec4&)>([&](xd::vec4& v) { return xd::normalize(v); }))
            .def_readwrite("x", &xd::vec4::x)
            .def_readwrite("y", &xd::vec4::y)
            .def_readwrite("z", &xd::vec4::z)
            .def_readwrite("w", &xd::vec4::w)
            .def_readwrite("r", &xd::vec4::r)
            .def_readwrite("g", &xd::vec4::g)
            .def_readwrite("b", &xd::vec4::b)
            .def_readwrite("a", &xd::vec4::a),
        // Aliases for creating a color
        def ("Color", tag_function<xd::vec4* (float, float, float, float)>(
            [](float r, float g, float b, float a) { return new xd::vec4(r, g, b, a); }
        ), adopt(result)),
        def ("Color", tag_function<xd::vec4* (float, float, float)>(
            [](float r, float g, float b) { return new xd::vec4(r, g, b, 1.0f); }
        ), adopt(result)),
        def ("Color", tag_function<xd::vec4* (std::string)>(
                [](std::string name) {
            if (name == "clear") {
                auto clear = hex_to_color(
                    Configurations::get<std::string>("startup.clear-color"));
                return new xd::vec4(clear);
            } else if (name == "none")
                return new xd::vec4;
            else if (name == "black")
                return new xd::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            else if (name == "red")
                return new xd::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            else if (name == "green")
                return new xd::vec4(0.0f, 1.0f, 0.0f, 1.0f);
            else if (name == "blue")
                return new xd::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            else if (name == "yellow")
                return new xd::vec4(1.0f, 1.0f, 0.0f, 1.0f);
            else if (name == "white")
                return new xd::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            else {
                try {
                    auto color = hex_to_color(name);
                    return new xd::vec4(color);
                } catch (std::runtime_error&) {}
            }
            return new xd::vec4;
        }), adopt(result)),
        // Direction utilities
        def("opposite_direction", tag_function<int (int)>(
            [](int dir) {
                return static_cast<int>(opposite_direction(static_cast<Direction>(dir)));
            }
        )),
        def("direction_to_vector", tag_function <xd::vec2 (int)>(
            [](int dir) {
                return direction_to_vector(static_cast<Direction>(dir));
            }
        )),
        def("vector_to_direction", tag_function<int (xd::vec2 vec)>(
            [](xd::vec2 vec) {
                return static_cast<int>(vector_to_direction(vec));
            }
        )),
        def("direction_to_string", tag_function<std::string (int)>(
            [](int dir) {
                return direction_to_string(static_cast<Direction>(dir));
            }
        )),
        def("string_to_direction", tag_function<int (const std::string&)>(
            [](const std::string& str) {
                return static_cast<int>(string_to_direction(str));
            }
        )),
        def("facing_direction", tag_function<int (xd::vec2, xd::vec2)>(
            [](xd::vec2 pos1, xd::vec2 pos2) {
                return static_cast<int>(facing_direction(pos1, pos2));
            }
        )),
        def("facing_direction", tag_function<int (xd::vec2, xd::vec2, bool)>(
            [](xd::vec2 pos1, xd::vec2 pos2, bool diagonal) {
                return static_cast<int>(facing_direction(pos1, pos2, diagonal));
            }
        )),
        def("is_diagonal", tag_function<bool (int)>(
            [](int dir) {
                return is_diagonal(static_cast<Direction>(dir));
            }
        )),
        // Rectangle
        class_<xd::rect>("Rect")
            .def(constructor<>())
            .def(constructor<const xd::rect&>())
            .def(constructor<float, float, float, float>())
            .def(constructor<const xd::vec2&, const xd::vec2&>())
            .def(constructor<const xd::vec2&, float, float>())
            .def(constructor<const xd::vec4&>())
            .def_readwrite("x", &xd::rect::x)
            .def_readwrite("y", &xd::rect::y)
            .def_readwrite("w", &xd::rect::w)
            .def_readwrite("h", &xd::rect::h)
            .def("intersects", &xd::rect::intersects),
        // Sprite holders: map object, image layer, canvas, etc.
        class_<Sprite_Holder>("Sprite_Holder")
            .def("reset", &Sprite_Holder::reset)
            .def("set_sprite", tag_function<void (Sprite_Holder*, const std::string&)>(
                [&](Sprite_Holder* holder, const std::string& filename) {
                    holder->set_sprite(*game, filename);
                }
            ))
            .def("set_sprite", tag_function<void (Sprite_Holder*, const std::string&, const std::string&)>(
                [&](Sprite_Holder* holder, const std::string& filename, const std::string& pose) {
                    holder->set_sprite(*game, filename, pose);
                }
            ))
            .def("show_pose", tag_function<Command_Result* (Sprite_Holder*, const std::string&)>(
                [&](Sprite_Holder* obj, const std::string& pose_name) {
                    return new Command_Result(std::make_shared<Show_Pose_Command>(obj, pose_name));
                }
            ), adopt(result))
            .def("show_pose", tag_function<Command_Result* (Sprite_Holder*, const std::string&, const std::string&)>(
                [&](Sprite_Holder* obj, const std::string& pose_name, const std::string& state) {
                    return new Command_Result(std::make_shared<Show_Pose_Command>(obj, pose_name, state));
                }
            ), adopt(result))
            .def("show_pose", tag_function<Command_Result* (Sprite_Holder*, const std::string&, const std::string&, Direction)>(
                [&](Sprite_Holder* obj, const std::string& pose_name, const std::string& state, Direction dir) {
                    return new Command_Result(std::make_shared<Show_Pose_Command>(obj, pose_name, state, dir));
                }
            ), adopt(result)),
        // Map object
        class_<Map_Object, Sprite_Holder>("Map_Object")
            .property("id", &Map_Object::get_id)
            .property("name", &Map_Object::get_name, &Map_Object::set_name)
            .property("type", &Map_Object::get_type, &Map_Object::set_type)
            .property("position", &Map_Object::get_position, &Map_Object::set_position)
            .property("x", &Map_Object::get_x, &Map_Object::set_x)
            .property("y", &Map_Object::get_y, &Map_Object::set_y)
            .property("color", &Map_Object::get_color, &Map_Object::set_color)
            .property("opacity", &Map_Object::get_opacity, &Map_Object::set_opacity)
            .property("disabled", &Map_Object::is_disabled, &Map_Object::set_disabled)
            .property("stopped", &Map_Object::is_stopped, &Map_Object::set_stopped)
            .property("frozen", &Map_Object::is_frozen, &Map_Object::set_frozen)
            .property("passthrough", &Map_Object::is_passthrough, &Map_Object::set_passthrough)
            .property("pose_name", &Map_Object::get_pose_name)
            .property("state", &Map_Object::get_state, &Map_Object::update_state)
            .property("walk_state", &Map_Object::get_walk_state, &Map_Object::set_walk_state)
            .property("face_state", &Map_Object::get_face_state, &Map_Object::set_face_state)
            .property("visible", &Map_Object::is_visible, &Map_Object::set_visible)
            .property("script", &Map_Object::get_trigger_script_source, &Map_Object::set_trigger_script_source)
            .property("triggered_object", &Map_Object::get_triggered_object, &Map_Object::set_triggered_object)
            .property("collision_object", &Map_Object::get_collision_object, &Map_Object::set_collision_object)
            .property("collision_area", &Map_Object::get_collision_area, &Map_Object::set_collision_area)
            .property("outlined", &Map_Object::is_outlined)
            .property("draw_order", &Map_Object::get_draw_order, &Map_Object::set_draw_order)
            .property("real_position", &Map_Object::get_real_position)
            .property("bounding_box", &Map_Object::get_bounding_box)
            .property("speed", &Map_Object::get_speed, &Map_Object::set_speed)
            .property("angle", &Map_Object::get_angle, &Map_Object::set_angle)
            .property("direction",
                tag_function<int (Map_Object*)>(
                    [](Map_Object* obj) {
                        return static_cast<int>(obj->get_direction());
                    }
                ),
                tag_function<void (Map_Object*, int)>(
                    [](Map_Object* obj, int dir) {
                        obj->set_direction(static_cast<Direction>(dir));
                    }
                )
            )
            .property("magnification", tag_function<xd::vec2 (Map_Object*)>(
                [](Map_Object* obj) -> xd::vec2 {
                    auto sprite = obj->get_sprite();
                    if (sprite)
                        return sprite->get_frame().magnification;
                    else
                        return xd::vec2(1.0f, 1.0f);
                }
            ))
            .def("get_property", &Map_Object::get_property)
            .def("set_property", &Map_Object::set_property)
            .def("move_to", tag_function<Command_Result* (Map_Object*, float, float)>(
                [&](Map_Object* obj, float x, float y) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Move_Object_To_Command>(
                            *game->get_map(),*obj, x, y
                        )
                    );
                }
            ), adopt(result))
            .def("move_to", tag_function<Command_Result* (Map_Object*, float, float, bool)>(
                [&](Map_Object* obj, float x, float y, bool keep_trying) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Move_Object_To_Command>(
                            *game->get_map(),*obj, x, y,
                            Collision_Check_Types::BOTH, keep_trying
                        )
                    );
                }
            ), adopt(result))
            .def("move", tag_function<Command_Result* (Map_Object*, int, float, bool, bool)>(
                [&](Map_Object* obj, int dir, float pixels, bool skip, bool change_facing) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Move_Object_Command>(
                            *obj, static_cast<Direction>(dir),
                            pixels, skip, change_facing
                        )
                    );
                }
            ), adopt(result))
            .def("move", tag_function<Command_Result* (Map_Object*, int, float, bool)>(
                [&](Map_Object* obj, int dir, float pixels, bool skip) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Move_Object_Command>(
                            *obj, static_cast<Direction>(dir), pixels, skip, true
                        )
                    );
                }
            ), adopt(result))
            .def("move", tag_function<Command_Result* (Map_Object*, int, float)>([&](Map_Object* obj, int dir, float pixels) {
                auto si = game->get_current_scripting_interface();
                return si->register_command(
                    std::make_shared<Move_Object_Command>(
                        *obj, static_cast<Direction>(dir), pixels, true, true
                    )
                );
            }), adopt(result))
            .def("face", (void (Map_Object::*)(xd::vec2)) &Map_Object::face)
            .def("face", (void (Map_Object::*)(float, float)) &Map_Object::face)
            .def("face", (void (Map_Object::*)(const Map_Object&)) &Map_Object::face)
            .def("face", tag_function<void (Map_Object*, int)>([&](Map_Object* obj, int dir) {
                obj->face(static_cast<Direction>(dir));
            }))
            .def("run_script", &Map_Object::run_trigger_script),
        // Game class
        class_<Game>("Game")
            .property("width", &Game::width)
            .property("height", &Game::height)
            .property("game_width", tag_function<float (Game*)>([](Game* game) { return game->game_width(); }))
            .property("game_height", tag_function<float (Game*)>([](Game* game) { return game->game_height(); }))
            .property("magnification", &Game::get_magnification, &Game::set_magnification)
            .property("ticks", &Game::ticks)
            .property("fps", &Game::fps)
            .property("frame_count", &Game::frame_count)
            .property("stopped", &Game::stopped)
            .property("seconds", &Game::seconds)
            .property("playing_music", tag_function<xd::music* (Game*)>(
                [](Game* game) {
                    return game->playing_music().get();
                }
            ))
            .def("set_size", &Game::set_size)
            .def("exit", &Game::exit)
            .def("pressed", tag_function<bool (Game*, const xd::key&)>(
                [](Game* game, const xd::key& key) {
                    return game->pressed(key);
                }
            ))
            .def("pressed", tag_function<bool (Game*, const std::string&)>(
                [](Game* game, const std::string& key) {
                    return game->pressed(key);
                }
            ))
            .def("triggered", tag_function<bool (Game*, const xd::key&)>(
                [](Game* game, const xd::key& key) {
                    return game->triggered(key);
                }
            ))
            .def("triggered", tag_function<bool (Game*, const std::string&)>(
                [](Game* game, const std::string& key) {
                    return game->triggered(key);
                }
            ))
            .def("triggered_once", tag_function<bool (Game*, const xd::key&)>(
                [](Game* game, const xd::key& key) {
                    return game->triggered_once(key);
                }
            ))
            .def("triggered_once", tag_function<bool (Game*, const std::string&)>(
                [](Game* game, const std::string& key) {
                    return game->triggered_once(key);
                }
            ))
            .def("run_script", &Game::run_script)
            .def("stop_time", tag_function<void (Game*)>([](Game* game) {
                game->get_clock()->stop_time();
            }))
            .def("resume_time", tag_function<void (Game*)>([](Game* game) {
                game->get_clock()->resume_time();
            }))
            .def("load_map", tag_function<void (Game*, const std::string&)>(
                [](Game* game, const std::string& filename) {
                    game->set_next_map(filename, Direction::NONE);
                }
            ))
            .def("load_map", tag_function<void (Game*, const std::string&, int)>(
                [](Game* game, const std::string& filename, int dir) {
                    game->set_next_map(filename, static_cast<Direction>(dir));
                }
            ))
            .def("load_map", tag_function<void (Game*, const std::string&, float, float, int)>(
                [](Game* game, const std::string& filename, float x, float y, int dir) {
                    game->set_next_map(filename, x, y, static_cast<Direction>(dir));
                }
            ))
            .def("get_config", tag_function<std::string (Game*, const std::string&)>(
                [](Game*, const std::string& key) {
                    return Configurations::get_string(key);
                }
            ))
            .def("save", tag_function<bool (Game*, const std::string&, object)>(
                [&](Game* game, const std::string& filename, object obj) {
                    Save_File file(vm.lua_state(), obj);
                    game->save(filename, file);
                    return file.is_valid();
                }
            ))
            .def("load", tag_function<luabind::object (Game*, const std::string&)>(
                [&](Game* game, const std::string& filename) {
                    auto file = game->load(filename);
                    if (file->is_valid())
                        return file->lua_data();
                    else
                        return luabind::object();
                }
            ))
            .def("load_music", tag_function<xd::music* (Game*, const std::string&)>(
                [&](Game* game, const std::string& filename) {
                    game->load_music(filename);
                    return game->playing_music().get();
                }
            ))
            .def("play_music", tag_function<void (Game*, const std::string&)>(
                [&](Game* game, const std::string& filename) {
                    game->load_music(filename);
                    game->playing_music()->play();
                }
            )),
        // Game map
        class_<Map>("Map")
            .property("width", &Map::get_width)
            .property("height", &Map::get_height)
            .property("tile_width", &Map::get_tile_width)
            .property("tile_height", &Map::get_tile_height)
            .property("filename", &Map::get_filename)
            .property("name", &Map::get_name)
            .def("get_property", &Map::get_property)
            .def("set_property", &Map::set_property)
            .def("object_count", &Map::object_count)
            .def("get_object", (Map_Object* (Map::*)(int)) &Map::get_object)
            .def("get_object", (Map_Object* (Map::*)(const std::string&)) &Map::get_object)
            .def("add_new_object", &Map::add_new_object)
            .def("add_object", tag_function<Map_Object* (Map*, Map_Object*, int)>(
                [](Map* map, Map_Object* object, int layer_index) {
                    return map->add_object(object, layer_index);
                }
            ))
            .def("delete_object", (void (Map::*)(Map_Object*)) &Map::delete_object)
            .def("layer_count", &Map::layer_count)
            .def("get_layer", (Layer* (Map::*)(int)) &Map::get_layer)
            .def("get_layer", (Layer* (Map::*)(const std::string&)) &Map::get_layer)
            .def("get_objects", tag_function<luabind::object (Map*)>([&](Map* map) -> luabind::object {
                luabind::object objects = luabind::newtable(vm.lua_state());
                int i = 1;
                auto& map_objects = map->get_objects();
                for (auto pair : map_objects) {
                    objects[i++] = pair.second.get();
                }
                return objects;
            }))
            .def("run_script", &Map::run_script)
            .def("passable", tag_function<bool (Map*, Map_Object*, int)>(
                [&](Map* map, Map_Object* object, int dir) {
                    auto c = map->passable(*object, static_cast<Direction>(dir));
                    return c.passable();
                }
            ))
            .def("colliding_object", tag_function<Map_Object* (Map*, Map_Object*)>(
                [&](Map* map, Map_Object* object) -> Map_Object* {
                    auto c = map->passable(*object, object->get_direction(),
                        Collision_Check_Types::OBJECT);
                    if (c.type == Collision_Types::OBJECT)
                        return c.other_object;
                    else
                        return nullptr;
                }
            )),
        // Game camera
        class_<Camera>("Camera")
            .property("tint_color", &Camera::get_tint_color,
                &Camera::set_tint_color)
            .property("position", &Camera::get_position, &Camera::set_position)
            .def("move", tag_function<Command_Result* (Camera*, int, float, float)>(
                [&](Camera* camera, int dir, float pixels, float speed) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Move_Camera_Command>(
                            *camera, static_cast<Direction>(dir), pixels, speed
                        )
                    );
                }
            ), adopt(result))
            .def("move_to", tag_function<Command_Result* (Camera*, float, float, float)>(
                [&](Camera* camera, float x, float y, float speed) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Move_Camera_Command>(*camera, x, y, speed)
                    );
                }
            ), adopt(result))
            .def("move_to", tag_function<Command_Result* (Camera*, Map_Object*, float)>(
                [&](Camera* camera, Map_Object* object, float speed) -> Command_Result* {
                    xd::vec2 position = object->get_position();
                    auto sprite = object->get_sprite();
                    float width = sprite ? sprite->get_size().x : 0.0f;
                    float height = sprite ? sprite->get_size().y : 0.0f;
                    float x = position.x + width / 2 - game->game_width() / 2;
                    float y = position.y + height / 2 - game->game_height() / 2;
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Move_Camera_Command>(*camera, x, y, speed)
                    );
                }
            ), adopt(result))
            .def("tint_screen", tag_function<Command_Result* (Camera*, xd::vec4, long)>(
                [&](Camera* camera, xd::vec4 color, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Tint_Screen_Command>(*game, color, duration)
                    );
                }
            ), adopt(result))
            .def("tint_screen", tag_function<Command_Result* (Camera*, const std::string&, long)>(
                [&](Camera* camera, const std::string& hex_color, long duration) {
                    auto si = game->get_current_scripting_interface();
                    auto color = hex_to_color(hex_color);
                    return si->register_command(
                        std::make_shared<Tint_Screen_Command>(*game, color, duration)
                    );
                }
            ), adopt(result))
            .def("zoom", tag_function<Command_Result* (Camera*, float, long)>(
                [&](Camera* camera, float scale, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Zoom_Command>(*game, scale, duration)
                    );
                }
            ), adopt(result))
            .def("center_at", (void (Camera::*)(xd::vec2)) &Camera::center_at)
            .def("center_at", (void (Camera::*)(const Map_Object&)) &Camera::center_at)
            .def("center_at", tag_function<void (Camera*, float, float)>(
                [](Camera* camera, float x, float y) {
                    camera->center_at(xd::vec2(x, y));
                }
            ))
            .def("track_object", tag_function<void (Camera*, Map_Object*)>([&](Camera* camera, Map_Object* object) {
                camera->set_object(object);
            }))
            .def("start_shaking", &Camera::start_shaking)
            .def("cease_shaking", &Camera::cease_shaking)
            .def("shake_screen", tag_function<Command_Result* (Camera*, float, float, long)>(
                [&](Camera* camera, float strength, float speed, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Shake_Screen_Command>(
                            *game, strength, speed, duration
                        )
                    );
                }
            ), adopt(result)),
        // Map layer
        class_<Layer>("Layer")
            .def_readonly("name", &Layer::name)
            .def_readwrite("visible", &Layer::visible)
            .def_readwrite("opacity", &Layer::opacity)
            .def("get_property", &Layer::get_property)
            .def("set_property", &Layer::set_property)
            .def("update_opacity", tag_function<Command_Result* (Layer*, float, long)>(
                    [&](Layer* layer, float opacity, long duration) {
                        auto si = game->get_current_scripting_interface();
                        return si->register_command(
                            std::make_shared<Update_Layer_Command>(
                                *game, *layer, opacity, duration
                            )
                        );
                    }
            ), adopt(result)),
        class_<Image_Layer, bases<Layer, Sprite_Holder>>("Image_Layer")
            .def_readwrite("velocity", &Image_Layer::velocity),
        // Sound effect
        class_<xd::sound>("Sound_Class")
            .property("playing", &xd::sound::playing)
            .property("paused", &xd::sound::paused)
            .property("stopped", &xd::sound::stopped)
            .property("offset", &xd::sound::get_offset, &xd::sound::set_offset)
            .property("volume", &xd::sound::get_volume, &xd::sound::set_volume)
            .property("pitch", &xd::sound::get_pitch, &xd::sound::set_pitch)
            .property("looping", &xd::sound::get_looping, &xd::sound::set_looping)
            .property("filename", &xd::sound::get_filename)
            .def("play", &xd::sound::play)
            .def("pause", &xd::sound::pause)
            .def("stop", &xd::sound::stop)
            .def("set_loop_points", &xd::sound::set_loop_points),
        def("Sound", tag_function<xd::sound* (const std::string&)>(
            [&](const std::string& filename) {
                return new xd::sound(*game->get_audio(), filename);
            }
        ), adopt(result)),
        // Background music
        class_<xd::music>("Music_Class")
            .property("playing", &xd::music::playing)
            .property("paused", &xd::music::paused)
            .property("stopped", &xd::music::stopped)
            .property("offset", &xd::music::get_offset, &xd::music::set_offset)
            .property("volume", &xd::music::get_volume, &xd::music::set_volume)
            .property("pitch", &xd::music::get_pitch, &xd::music::set_pitch)
            .property("looping", &xd::music::get_looping, &xd::music::set_looping)
            .property("filename", &xd::music::get_filename)
            .def("play", &xd::music::play)
            .def("pause", &xd::music::pause)
            .def("stop", &xd::music::stop)
            .def("set_loop_points", &xd::music::set_loop_points)
            .def("fade", tag_function<Command_Result* (xd::music*, float, long)>(
                [&](xd::music* music, float volume, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Fade_Music_Command>(
                            *game, volume, duration
                        )
                    );
                }
            ), adopt(result)),
        // Parsed text token
        class_<Token>("Token")
            .def_readonly("type", &Token::type)
            .def_readonly("tag", &Token::tag)
            .def_readonly("value", &Token::value)
            .def_readonly("unmatched", &Token::unmatched)
            .def_readonly("start_index", &Token::start_index)
            .def_readonly("end_index", &Token::end_index),
        // Canvas parser
        class_<Text_Parser>("Text_Parser")
            .def(constructor<>())
            .def("parse", tag_function<luabind::object (Text_Parser&, const std::string&, bool)>(
                [&](Text_Parser& parser, const std::string& text, bool permissive) {
                    luabind::object objects = luabind::newtable(vm.lua_state());
                    auto tokens = parser.parse(text, permissive);
                    for (size_t i = 1; i <= tokens.size(); ++i) {
                        objects[i] = tokens[i - 1];
                    }
                    return objects;
                }
            )),
        // A drawing canvas
        class_<Canvas, Sprite_Holder, std::shared_ptr<Sprite_Holder>>("Canvas_Class")
            .property("name", &Canvas::get_name, &Canvas::set_name)
            .property("priority", &Canvas::get_priority, &Canvas::set_priority)
            .property("position", &Canvas::get_position, &Canvas::set_position)
            .property("x", &Canvas::get_x, &Canvas::set_x)
            .property("y", &Canvas::get_y, &Canvas::set_y)
            .property("pose_name", &Canvas::get_pose_name)
            .property("pose_state", &Canvas::get_pose_state)
            .property("pose_direction", &Canvas::get_pose_direction)
            .property("origin", &Canvas::get_origin, &Canvas::set_origin)
            .property("magnification", &Canvas::get_magnification, &Canvas::set_magnification)
            .property("scissor_box", &Canvas::get_scissor_box, &Canvas::set_scissor_box)
            .property("angle", &Canvas::get_angle, &Canvas::set_angle)
            .property("opacity", &Canvas::get_opacity, &Canvas::set_opacity)
            .property("color", &Canvas::get_color, &Canvas::set_color)
            .property("filename", &Canvas::get_filename)
            .property("text", &Canvas::get_text, &Canvas::set_text)
            .property("width", &Canvas::get_width)
            .property("height", &Canvas::get_height)
            .property("font_size", &Canvas::get_font_size, &Canvas::set_font_size)
            .property("text_color", &Canvas::get_text_color, &Canvas::set_text_color)
            .property("line_height", &Canvas::get_line_height, &Canvas::set_line_height)
            .property("text_outline_width", &Canvas::get_text_outline_width, &Canvas::set_text_outline_width)
            .property("text_outline_color", &Canvas::get_text_outline_color, &Canvas::set_text_outline_color)
            .property("text_shadow_offset", &Canvas::get_text_shadow_offset, &Canvas::set_text_shadow_offset)
            .property("text_shadow_color", &Canvas::get_text_shadow_color, &Canvas::set_text_shadow_color)
            .property("text_type", &Canvas::get_text_type, &Canvas::set_text_type)
            .property("child_count", &Canvas::get_child_count)
            .property("permissive_tag_parsing", &Canvas::get_permissive_tag_parsing, &Canvas::set_permissive_tag_parsing)
            .property("has_image_outline", &Canvas::has_image_outline, &Canvas::set_image_outline)
            .property("image_outline_color", &Canvas::get_image_outline_color, &Canvas::set_image_outline_color)
            .def("reset_text_outline", &Canvas::reset_text_outline)
            .def("reset_text_shadow", &Canvas::reset_text_shadow)
            .def("reset_text_type", &Canvas::reset_text_type)
            .def("set_font", &Canvas::set_font)
            .def("link_font", &Canvas::link_font)
            .def("remove_child", &Canvas::remove_child)
            .def("get_child", (Canvas* (Canvas::*)(std::size_t)) &Canvas::get_child)
            .def("get_child", (Canvas* (Canvas::*)(const std::string&)) &Canvas::get_child)
            .def("show", tag_function<void (Canvas*)>([](Canvas* canvas) {
                canvas->set_visible(true);
            }))
            .def("hide", tag_function<void (Canvas*)>([](Canvas* canvas) {
                canvas->set_visible(false);
            }))
            .def("set_image", tag_function<void (Canvas*, const std::string&)>(
                [](Canvas* canvas, const std::string& filename) {
                    canvas->set_image(filename);
                }
            ))
            .def("set_image", tag_function<void (Canvas*, const std::string&, xd::vec4)>(
                [](Canvas* canvas, const std::string& filename, xd::vec4 ck) {
                    canvas->set_image(filename, ck);
                }
            ))
            .def("update", tag_function<Command_Result* (Canvas*, float, float,
                    float, float, float, float, long)>(
                [&](Canvas* canvas, float x, float y, float mag_x,
                        float mag_y, float angle, float opacity, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Update_Canvas_Command>(
                            *game, *canvas, duration, xd::vec2(x, y),
                            xd::vec2(mag_x, mag_y), angle, opacity
                        )
                    );
                }
            ), adopt(result))
            .def("move", tag_function<Command_Result* (Canvas*, float, float, long)>(
                [&](Canvas* canvas, float x, float y, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Update_Canvas_Command>(
                            *game, *canvas, duration,
                            xd::vec2(x, y), canvas->get_magnification(),
                            canvas->get_angle(), canvas->get_opacity()
                        )
                    );
                }
            ), adopt(result))
            .def("resize", tag_function<Command_Result* (Canvas*, float, float, long)>(
                [&](Canvas* canvas, float mag_x, float mag_y, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Update_Canvas_Command>(
                            *game, *canvas, duration,
                            canvas->get_position(), xd::vec2(mag_x, mag_y),
                            canvas->get_angle(), canvas->get_opacity()
                        )
                    );
                }
            ), adopt(result))
            .def("update_opacity", tag_function<Command_Result* (Canvas*, float, long)>(
                [&](Canvas* canvas, float opacity, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Update_Canvas_Command>(
                            *game, *canvas, duration, canvas->get_position(),
                            canvas->get_magnification(), canvas->get_angle(), opacity
                        )
                    );
                }
            ), adopt(result))
            .def("rotate", tag_function<Command_Result* (Canvas*, float, long)>(
                [&](Canvas* canvas, float angle, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Update_Canvas_Command>(
                            *game, *canvas, duration, canvas->get_position(),
                            canvas->get_magnification(), angle, canvas->get_opacity()
                        )
                    );
                }
            ), adopt(result))
            .def("add_child_image", tag_function<Canvas* (Canvas*, const std::string&, const std::string&, float, float)>(
                [&](Canvas* parent, const std::string& name, const std::string& filename, float x, float y) -> Canvas* {
                    auto extension = filename.substr(filename.find_last_of(".") + 1);
                    if (extension == "spr")
                        return parent->add_child(name, *game, filename, "", xd::vec2(x, y));
                    else
                        return parent->add_child(name, filename, xd::vec2(x, y));
                }
            ))
            // Add child (with hex trans color or sprite with pose name)
            .def("add_child_image", tag_function<Canvas* (Canvas*, const std::string&, const std::string&, float, float, const std::string&)>(
                [&](Canvas* parent, const std::string& name, const std::string& filename, float x, float y, const std::string& trans_or_pose) -> Canvas* {
                    auto extension = filename.substr(filename.find_last_of(".") + 1);
                    if (extension == "spr")
                        return parent->add_child(name, *game, filename, trans_or_pose, xd::vec2(x, y));
                    else
                        return parent->add_child(name, filename, xd::vec2(x, y), hex_to_color(trans_or_pose));
                }
            ))
            // Add child (with transparent color as vec4)
            .def("add_child_image", tag_function<Canvas* (Canvas*, const std::string&, const std::string&, float, float, const xd::vec4&)>(
                [&](Canvas* parent, const std::string& name, const std::string& filename, float x, float y, const xd::vec4& trans) -> Canvas* {
                    return parent->add_child(name, filename, xd::vec2(x, y), trans);
                }
            ))
            // Add child (with text and position)
            .def("add_child_text", tag_function<Canvas* (Canvas*, const std::string&, float, float, const std::string&)>(
                [&](Canvas* parent, const std::string& name, float x, float y, const std::string& text) -> Canvas* {
                    return parent->add_child(name, *game, xd::vec2(x, y), text, true);
                }
            )),
        // Canvas constructor
        def("Canvas", tag_function<std::shared_ptr<Sprite_Holder> (const std::string&, float, float)>(
            [&](const std::string& filename, float x, float y) -> std::shared_ptr<Sprite_Holder> {
                std::shared_ptr<Canvas> canvas;
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    canvas = std::make_shared<Canvas>(*game, filename, "", xd::vec2(x, y));
                else
                    canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y));
                game->add_canvas(canvas);
                return canvas;
            }
        )),
        // Canvas constructor (with transparent color as vec4)
        def("Canvas", tag_function<std::shared_ptr<Sprite_Holder>(const std::string&, float, float, const xd::vec4&)>(
            [&](const std::string& filename, float x, float y, const xd::vec4& trans) -> std::shared_ptr<Sprite_Holder> {
                auto canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y), trans);
                game->add_canvas(canvas);
                return canvas;
            }
        )),
        // Canvas constructor (with hex trans color or sprite with pose name)
        def("Canvas", tag_function<std::shared_ptr<Sprite_Holder>(const std::string&, float, float, const std::string&)>(
            [&](const std::string& filename, float x, float y, const std::string& trans_or_pose) -> std::shared_ptr<Sprite_Holder> {
               std::shared_ptr<Canvas> canvas;
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    canvas = std::make_shared<Canvas>(*game, filename, trans_or_pose, xd::vec2(x, y));
                else
                    canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y), hex_to_color(trans_or_pose));
                game->add_canvas(canvas);
                return canvas;
            }
        )),
        // Canvas constructor (with text and position)
        def("Canvas", tag_function<std::shared_ptr<Sprite_Holder> (float, float, const std::string&)>(
            [&](float x, float y, const std::string& text) -> std::shared_ptr<Sprite_Holder> {
                auto canvas = std::make_shared<Canvas>(*game, xd::vec2(x, y), text);
                game->add_canvas(canvas);
                return canvas;
            }
        )),
        // Options for displaying text
        class_<Text_Options>("Text_Options")
            .def(constructor<>())
            .def(constructor<Map_Object*>())
            .def(constructor<xd::vec2>())
            .def_readonly("text", &Text_Options::text)
            .def_readonly("choices", &Text_Options::choices)
            .def_readonly("object", &Text_Options::object)
            .def_readonly("position", &Text_Options::position)
            .def_readonly("position_type", &Text_Options::position_type)
            .def_readonly("duration", &Text_Options::duration)
            .def_readonly("centered", &Text_Options::centered)
            .def_readonly("show_dashes", &Text_Options::show_dashes)
            .def_readonly("choice_indent", &Text_Options::choice_indent)
            .def_readonly("translated", &Text_Options::translated)
            .def_readonly("canvas_priority", &Text_Options::canvas_priority)
            .def_readonly("fade_in_duration", &Text_Options::fade_in_duration)
            .def_readonly("fade_out_duration", &Text_Options::fade_out_duration)
            .def("set_text", &Text_Options::set_text)
            .def("set_choices", tag_function<Text_Options& (Text_Options*, const object&)>(
                [&](Text_Options* options, const object& table) -> Text_Options& {
                    std::vector<std::string> choices;
                    if (type(table) == LUA_TTABLE) {
                        for (iterator i(table), end; i != end; ++i) {
                            choices.push_back(object_cast<std::string>(*i));
                        }
                    }
                    return options->set_choices(choices);
                }
            ))
            .def("set_object", &Text_Options::set_object)
            .def("set_position", &Text_Options::set_position)
            .def("set_position_type", tag_function<Text_Options& (Text_Options*, int)>(
                [&](Text_Options* options, int type) -> Text_Options& {
                    return options->set_position_type(static_cast<Text_Position_Type>(type));
                }
            ))
            .def("set_duration", &Text_Options::set_duration)
            .def("set_centered", &Text_Options::set_centered)
            .def("set_show_dashes", &Text_Options::set_show_dashes)
            .def("set_translated", &Text_Options::set_translated)
            .def("set_choice_indent", &Text_Options::set_choice_indent)
            .def("set_canvas_priority", &Text_Options::set_canvas_priority)
            .def("set_fade_in_duration", &Text_Options::set_fade_in_duration)
            .def("set_fade_out_duration", &Text_Options::set_fade_out_duration),
        // Show some text
        def("text", tag_function<Command_Result* (const Text_Options&)>(
                [&](const Text_Options& options) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(*game, options));
                }
        ), adopt(result)),
        def("text", tag_function<Command_Result* (Map_Object&, const std::string&)>(
                [&](Map_Object& obj, const std::string& text) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, Text_Options{&obj}.set_text(text)
                    ));
                }
        ), adopt(result)),
        def("text", tag_function<Command_Result* (xd::vec2&, const std::string&)>(
                [&](xd::vec2& position, const std::string& text) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, Text_Options{position}.set_text(text)
                    ));
                }
        ), adopt(result)),
        def("centered_text", tag_function<Command_Result* (float, const std::string&)>(
                [&](float y, const std::string& text) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, Text_Options{xd::vec2{0.0f, y}}.set_text(text).set_centered(true)
                    ));
                }
        ), adopt(result)),
        // Show timed text
        def("text", tag_function<Command_Result* (Map_Object&, const std::string&, long)>(
                [&](Map_Object& obj, const std::string& text, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, Text_Options{&obj}.set_text(text).set_duration(duration)
                    ));
                }
        ), adopt(result)),
        def("text", tag_function<Command_Result* (xd::vec2&, const std::string&, long)>(
                [&](xd::vec2& position, const std::string& text, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, Text_Options{position}.set_text(text).set_duration(duration)
                    ));
                }
        ), adopt(result)),
        def("centered_text", tag_function<Command_Result* (float, const std::string&, long)>(
                [&](float y, const std::string& text, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, Text_Options{ xd::vec2{0.0f, y} }.set_text(text).set_duration(duration).set_centered(true)
                    ));
                }
        ), adopt(result)),
        // Like Command_Result but stores the index of selected choice
        class_<Choice_Result>("Choice_Result")
            .def("is_complete", &Choice_Result::operator())
            .def("wait", tag_function<void (Choice_Result*)>(result_wait), yield)
            .property("selected", tag_function<int (Choice_Result*)>(
                [](Choice_Result* cr) { return cr->choice_index() + 1; })),
        // Show some text followed by a list of choices
        def("choices", tag_function<Choice_Result* (const Text_Options&)>(
                [&](const Text_Options& options) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_choice_command(std::make_shared<Show_Text_Command>(*game, options));
                }
        ), adopt(result)),
        def("choices",
            tag_function<Choice_Result* (Map_Object&, const std::string&, const object&)>(
                [&](Map_Object& obj, const std::string& text, const object& table) {
                    std::vector<std::string> choices;
                    if (type(table) == LUA_TTABLE) {
                        for (iterator i(table), end; i != end; ++i) {
                            choices.push_back(object_cast<std::string>(*i));
                        }
                    }
                    auto command = std::make_shared<Show_Text_Command>(
                        *game, Text_Options{&obj}.set_choices(choices).set_text(text));
                    auto si = game->get_current_scripting_interface();
                    return si->register_choice_command(command);
                }
        ), adopt(result)),
        def("choices",
            tag_function<Choice_Result* (xd::vec2&, const std::string&, const object&)>(
                [&](xd::vec2& position, const std::string& text, const object& table) {
                    std::vector<std::string> choices;
                    if (type(table) == LUA_TTABLE) {
                        for (iterator i(table), end; i != end; ++i) {
                            choices.push_back(object_cast<std::string>(*i));
                        }
                    }
                    auto command = std::make_shared<Show_Text_Command>(
                        *game, Text_Options{position}.set_choices(choices).set_text(text));
                    auto si = game->get_current_scripting_interface();
                    return si->register_choice_command(command);
                }
        ), adopt(result))
    ];
}
