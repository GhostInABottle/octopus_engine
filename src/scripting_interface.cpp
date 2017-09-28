#include "../include/scripting_interface.hpp"
#include "../include/game.hpp"
#include "../include/clock.hpp"
#include "../include/npc.hpp"
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
#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/tag_function.hpp>
#include <xd/audio.hpp>

Game* Scripting_Interface::game = nullptr;
xd::lua::virtual_machine Scripting_Interface::vm;

void Scripting_Interface::update() {
    if (scheduler.pending_tasks() > 0)
        scheduler.run();
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
}

void Scripting_Interface::run_script(const std::string& script) {
    if (!script.empty())
        scheduler.start(vm.load(script));
}

void Scripting_Interface::set_globals() {
    // Global variables
    vm.globals()["game"] = game;
    vm.globals()["current_map"] = game->get_map();
    vm.globals()["camera"] = game->get_camera();
    vm.globals()["player"] = game->get_player();
    // Resolution
    vm.globals()["game_width"] = Game::game_width;
    vm.globals()["game_height"] = Game::game_height;
    vm.globals()["screen_width"] = game->width();
    vm.globals()["screen_height"] = game->height();
    // Directions
    vm.globals()["UP"] = 1;
    vm.globals()["RIGHT"] = 2;
    vm.globals()["DOWN"] = 4;
    vm.globals()["LEFT"] = 8;
    vm.globals()["FORWARD"] = 16;
    vm.globals()["BACKWARD"] = 32;
    // Object draw order
    vm.globals()["DRAW_BELOW"] = 0;
    vm.globals()["DRAW_NORMAL"] = 1;
    vm.globals()["DRAW_ABOVE"] = 2;
}

Command_Result* Scripting_Interface::register_command(std::shared_ptr<Command> command) {
    commands.push_back(command);
    return new Command_Result(command);
}

Choice_Result* Scripting_Interface::register_choice_command(std::shared_ptr<Command> command) {
    commands.push_back(command);
    return new Choice_Result(command);
}


void Scripting_Interface::setup_scripts() {
    using namespace luabind;
    vm.load_library();
    luaL_openlibs(vm.lua_state());
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
    auto get_npcs = [](Game* game, bool on_map_only) -> luabind::object {
        luabind::object objects = luabind::newtable(vm.lua_state());
        auto& npcs = game->get_npcs(); 
        int i = 1;
        for (auto& npc : npcs) {
            if (!on_map_only || (npc->get_object() &&
                    npc->get_map_name() == game->get_map()->get_filename()))
                objects[i++] = npc.get();
        }
        return objects;
    };

    module(vm.lua_state())
    [
        // Returned from commands that allow yielding
        class_<Command_Result>("Command_Result")
            .def("is_complete", &Command_Result::operator())
            .def("wait", tag_function<void (Command_Result*)>(result_wait), yield),
        def("wait", tag_function<void (int)>([&](int duration) {
            wait(*game, duration); 
        }), yield),
        def("wait", tag_function<void (const std::string&)>([&](const std::string& key) {
            wait_press(*game, key); 
        }), yield),
        def("time_to_days", &time_to_days),
        def("time_to_hours", &time_to_hours),
        def("time_to_minutes", &time_to_minutes),
        def("time_to_seconds", &time_to_seconds),
        def("time_without_days", &time_without_days),
        def("text_width", tag_function<float (const std::string&)>(
            [&](const std::string& text) {
            return game->get_font()->get_width(text,
                xd::font_style(xd::vec4(1.0f, 1.0f, 1.0f, 1.0f), 8)
                .force_autohint(true));
            }
        )),
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
            .property("state", &Map_Object::get_state)
            .property("visible", &Map_Object::is_visible, &Map_Object::set_visible)
            .property("script", &Map_Object::get_trigger_script_source, &Map_Object::set_trigger_script_source)
            .property("triggered_object", &Map_Object::get_triggered_object, &Map_Object::set_triggered_object)
            .property("collision_area", &Map_Object::get_collision_area, &Map_Object::set_collision_area)
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
            .def("get_magnification", tag_function<xd::vec2 (Map_Object*)>( 
                [](Map_Object* obj) -> xd::vec2 {
                    auto sprite = obj->get_sprite();
                    if (sprite)
                        return sprite->get_frame().magnification;
                    else
                        return xd::vec2(1.0f, 1.0f);
                }
            ))
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
            .property("ticks", &Game::ticks)
            .property("fps", &Game::fps)
            .property("frame_count", &Game::frame_count)
            .property("stopped", &Game::stopped)
            .property("total_seconds", &Game::total_seconds)
            .def("pressed", (bool (Game::*)(const xd::key&, int) const) &Game::pressed)
            .def("pressed", tag_function<bool (Game*, const xd::key&)>(
                [](Game* game, const xd::key& key) {
                    return game->pressed(key);
            }))
            .def("pressed", (bool (Game::*)(const std::string&, int) const) &Game::pressed)
            .def("pressed", tag_function<bool (Game*, const std::string&)>(
                [](Game* game, const std::string& key) {
                    return game->pressed(key);
            }))
            .def("triggered", (bool (Game::*)(const xd::key&, int) const) &Game::triggered)
            .def("triggered", tag_function<bool (Game*, const xd::key&)>(
                [](Game* game, const xd::key& key) {
                    return game->triggered(key);
            }))
            .def("triggered", (bool (Game::*)(const std::string&, int) const) &Game::triggered)
            .def("triggered", tag_function<bool (Game*, const std::string&)>(
                [](Game* game, const std::string& key) {
                    return game->triggered(key);
            }))
            .def("triggered_once", (bool (Game::*)(const xd::key&, int)) &Game::triggered_once)
            .def("triggered_once", tag_function<bool (Game*, const xd::key&)>(
                [](Game* game, const xd::key& key) {
                    return game->triggered_once(key);
            }))
            .def("triggered_once", (bool (Game::*)(const std::string&, int)) &Game::triggered_once)
            .def("triggered_once", tag_function<bool (Game*, const std::string&)>(
                [](Game* game, const std::string& key) {
                    return game->triggered_once(key);
            }))
            .def("playing_music", &Game::playing_music)
            .def("run_script", &Game::run_script)
            .def("stop_time", tag_function<void (Game*)>([](Game* game) {
                game->get_clock()->stop_time();
            }))
            .def("resume_time", tag_function<void (Game*)>([](Game* game) {
                game->get_clock()->resume_time();
            }))
            .def("add_seconds", tag_function <void (Game*, int)>(
                [](Game* game, int seconds) {
                    game->get_clock()->add_seconds(seconds);
                }
            ))
            .def("get_npc", &Game::get_npc)
            .def("get_npcs", tag_function<luabind::object (Game*)>([&](Game* game) {
                return get_npcs(game, false);
            }))
            .def("get_map_npcs", tag_function<luabind::object (Game*)>(
                [&](Game* game) {
                    return get_npcs(game, true);
                }
            ))
            .def("load_map", tag_function<void (Game*, const std::string&, float, float, int)>(
                [](Game* game, const std::string& filename, float x, float y, int dir) {
                    game->set_next_map(filename, x, y, static_cast<Direction>(dir));
                })
            )
            .def("get_config", tag_function<std::string (Game*, const std::string&)>(
                [](Game*, const std::string& key) {
                    return Configurations::get_string(key);
                }
            ))
            .def("save", tag_function<void (Game*, const std::string&, object)>(
                [&](Game* game, const std::string& filename, object obj) {
                    Save_File file(vm.lua_state(), obj);
                    game->save(filename, file);
                }
            ))
            .def("load", tag_function<luabind::object (Game*, const std::string&)>(
                [&](Game* game, const std::string& filename) {
                    return game->load(filename)->lua_data();
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
            .def("object_count", &Map::object_count)
            .def("get_object", (Map_Object* (Map::*)(int)) &Map::get_object)
            .def("get_object", (Map_Object* (Map::*)(const std::string&)) &Map::get_object)
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
            .property("position", &Camera::get_position)
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
                    float x = position.x + width / 2 - game->game_width / 2;
                    float y = position.y + height / 2 - game->game_height / 2;
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
                [&](Camera* camera, const std::string& color, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Tint_Screen_Command>(
                            *game, hex_to_color(color), duration
                        )
                    );
                }
            ), adopt(result))
            .def("center_at", &Camera::center_at)
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
            .def("play", &xd::sound::play)
            .def("pause", &xd::sound::pause)
            .def("stop", &xd::sound::stop)
            .def("set_loop_points", &xd::sound::set_loop_points),
        def("Sound", tag_function<xd::sound::ptr (const std::string&)>(
            [&](const std::string& filename) {
                return game->load_sound(filename);
            }
        )),
        // Background music
        class_<xd::music>("Music_Class")
            .property("playing", &xd::music::playing)
            .property("paused", &xd::music::paused)
            .property("stopped", &xd::music::stopped)
            .property("offset", &xd::music::get_offset, &xd::music::set_offset)
            .property("volume", &xd::music::get_volume, &xd::music::set_volume)
            .property("pitch", &xd::music::get_pitch, &xd::music::set_pitch)
            .property("looping", &xd::music::get_looping, &xd::music::set_looping)
            .def("play", &xd::music::play)
            .def("pause", &xd::music::pause)
            .def("stop", &xd::music::stop)
            .def("set_loop_points", &xd::music::set_loop_points)
            .def("fade", tag_function<Command_Result* (xd::music*, float, long)>(
                [&](xd::music* music, float volume, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(
                        std::make_shared<Fade_Music_Command>(
                            *game, *music, volume, duration
                        )
                    );
                }
            ), adopt(result)),
        def("Music", tag_function<xd::music* (const std::string&)>(
            [&](const std::string& filename) {
                return game->load_music(filename).get();
            }
        )),
        // A drawing canvas
        class_<Canvas, Sprite_Holder>("Canvas_Class")
            .property("position", &Canvas::get_position, &Canvas::set_position)
            .property("x", &Canvas::get_x, &Canvas::set_x)
            .property("y", &Canvas::get_y, &Canvas::set_y)
            .property("origin", &Canvas::get_origin, &Canvas::set_origin)
            .property("magnification", &Canvas::get_magnification, &Canvas::set_magnification)
            .property("angle", &Canvas::get_angle, &Canvas::set_angle)
            .property("opacity", &Canvas::get_opacity, &Canvas::set_opacity)
            .property("filename", &Canvas::get_filename)
            .property("text", &Canvas::get_text, &Canvas::set_text)
            .property("width", &Canvas::get_width)
            .property("height", &Canvas::get_height)
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
            ), adopt(result)),
        // Canvas constructor
        def("Canvas", tag_function<Canvas* (const std::string&, float, float)>(
            [&](const std::string& filename, float x, float y) -> Canvas* {
                std::shared_ptr<Canvas> canvas;
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    canvas = std::make_shared<Canvas>(*game, filename, "", xd::vec2(x, y));
                else
                    canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y));
                game->add_canvas(canvas);
                return canvas.get();
            }
        )),
        // Canvas constructor (with transparent color as vec4)
        def("Canvas", tag_function<Canvas* (const std::string&, float, float, const xd::vec4&)>(
            [&](const std::string& filename, float x, float y, const xd::vec4& trans) -> Canvas* {
                auto canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y), trans);
                game->add_canvas(canvas);
                return canvas.get();
            }
        )),
        // Canvas constructor (with hex trans color or sprite with pose name)
        def("Canvas", tag_function<Canvas* (const std::string&, float, float, const std::string&)>(
            [&](const std::string& filename, float x, float y, const std::string& trans_or_pose) -> Canvas* {
               std::shared_ptr<Canvas> canvas;
                auto extension = filename.substr(filename.find_last_of(".") + 1);
                if (extension == "spr")
                    canvas = std::make_shared<Canvas>(*game, filename, trans_or_pose, xd::vec2(x, y));
                else
                    canvas = std::make_shared<Canvas>(filename, xd::vec2(x, y), hex_to_color(trans_or_pose));
                game->add_canvas(canvas);
                return canvas.get();
            }
        )),
        // Canvas constructor (with text and position)
        def("Canvas", tag_function<Canvas* (float, float, const std::string&)>(
            [&](float x, float y, const std::string& text) -> Canvas* {
                auto canvas = std::make_shared<Canvas>(*game, xd::vec2(x, y), text);
                game->add_canvas(canvas);
                return canvas.get();
            }
        )),
        // A scheduled NPC
        class_<NPC>("NPC")
            .property("name", &NPC::get_name)
            .property("display_name", &NPC::get_display_name)
            .property("active", &NPC::is_active, &NPC::set_active)
            .property("map_name", &NPC::get_map_name)
            .property("position", &NPC::get_position)
            .property("object", &NPC::get_object)
            .property("keypoint_day", &NPC::keypoint_day)
            .property("keypoint_time", &NPC::keypoint_time)
            .property("keypoint_day_type", &NPC::keypoint_day_type)
            .property("schedule", &NPC::get_schedule, &NPC::set_schedule)
            .def("has_schedule", &NPC::has_schedule),
        // Show some text
        def("text", tag_function<Command_Result* (Map_Object&, const std::string&)>(
                [&](Map_Object& obj, const std::string& text) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, &obj, text
                    ));
                }
        ), adopt(result)),
        def("text", tag_function<Command_Result* (xd::vec2&, const std::string&)>(
                [&](xd::vec2& position, const std::string& text) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, position, std::vector<std::string>{}, text
                    ));
                }
        ), adopt(result)),
        def("centered_text", tag_function<Command_Result* (float, const std::string&)>(
                [&](float y, const std::string& text) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, xd::vec2{0.0f, y}, std::vector<std::string>{}, text, -1, true
                    ));
                }
        ), adopt(result)),
        // Show timed text
        def("text", tag_function<Command_Result* (Map_Object&, const std::string&, long)>(
                [&](Map_Object& obj, const std::string& text, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, &obj, text, duration
                    ));
                }
        ), adopt(result)),
        def("text", tag_function<Command_Result* (xd::vec2&, const std::string&, long)>(
                [&](xd::vec2& position, const std::string& text, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, position, std::vector<std::string>{}, text, duration
                    ));
                }
        ), adopt(result)),
        def("centered_text", tag_function<Command_Result* (float, const std::string&, long)>(
                [&](float y, const std::string& text, long duration) {
                    auto si = game->get_current_scripting_interface();
                    return si->register_command(std::make_shared<Show_Text_Command>(
                        *game, xd::vec2{0.0f, y}, std::vector<std::string>{}, text, duration, true
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
                        *game, &obj, choices, text);
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
                        *game, position, choices, text);
                    auto si = game->get_current_scripting_interface();
                    return si->register_choice_command(command);
                }
        ), adopt(result))
    ];
}
