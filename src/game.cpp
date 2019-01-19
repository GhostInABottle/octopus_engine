#include "../include/game.hpp"
#include "../include/clock.hpp"
#include "../include/player_controller.hpp"
#include "../include/camera.hpp"
#include "../include/object_layer.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/scripting_interface.hpp"
#include "../include/command.hpp"
#include "../include/configurations.hpp"
#include "../include/utility.hpp"
#include "../include/custom_shaders.hpp"
#include "../include/save_file.hpp"
#include "../include/log.hpp"
#include <xd/graphics.hpp>
#include <xd/factory.hpp>
#include <xd/asset_manager.hpp>
#include <xd/lua/virtual_machine.hpp>
#include <luabind/luabind.hpp>
#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

int Game::game_width;
int Game::game_height;

struct Game::Impl {
    explicit Impl(bool editor_mode) :
            editor_mode(editor_mode),
            show_fps(Configurations::get<bool>("debug.show-fps")),
            show_time(Configurations::get<bool>("debug.show-time")),
            pause_unfocused(Configurations::get<bool>("game.pause-unfocused")),
            paused(false),
            focus_pause(false),
            music_was_paused(false),
            was_stopped(false),
            pause_start_time(0),
            total_paused_time(0),
            exit_requested(false),
            current_shader(nullptr),
            debug_style(xd::vec4(1.0f), Configurations::get<int>("font.size")){}
    std::unique_ptr<Scripting_Interface> scripting_interface;
    bool show_fps;
    bool show_time;
    // Was game started in editor mode?
    bool editor_mode;
    // Information for teleporting to another map
    xd::vec2 next_position;
    Direction next_direction;
    std::string next_map;
    // Is the game paused?
    bool paused;
    // Was it paused because screen got unfocused?
    bool focus_pause;
    // was music already paused when game got paused?
    bool music_was_paused;
    // Is pausing when screen is unfocused enabled?
    bool pause_unfocused;
    // Was time stopped when game got paused?
    bool was_stopped;
    // Keep track of paused time
    int pause_start_time;
    int total_paused_time;
    // Is it time to exit the main loop?
    bool exit_requested;
    // Full-screen shader data
    xd::shader_program* current_shader;
    xd::sprite_batch full_screen_batch;
    xd::texture::ptr full_screen_texture;
    // Texture asset manager
    xd::asset_manager asset_manager;
    // The shared Lua virtual machine
    xd::lua::virtual_machine vm;
    // Debug font style (FPS and time display)
    xd::font_style debug_style;
    // Render a full-screen shader
    void render_shader(Game& game);
};

Game::Game(bool editor_mode) :
        window(editor_mode ? nullptr : new xd::window(
            Configurations::get<std::string>("game.title"),
            Configurations::get<int>("game.screen-width"),
            Configurations::get<int>("game.screen-height"),
            xd::window_options(Configurations::get<bool>("game.fullscreen"),
                false, false, false, 8, 0, 0, 2, 0))),
        style(xd::vec4(1.0f, 1.0f, 1.0f, 1.0f), Configurations::get<int>("font.size")),
        pimpl(new Impl(editor_mode)),
        current_scripting_interface(nullptr),
        text_renderer(static_cast<float>(game_width), static_cast<float>(game_height)) {
    xd::audio::init();
    clock.reset(new Clock(*this));
    camera.reset(new Camera(*this));
    // Setup fonts
    style.outline(1, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f))
        .line_height(12.0f).force_autohint(true);
    pimpl->debug_style.line_height(12.0f).force_autohint(true);
    auto font_file = Configurations::get<std::string>("font.default");
    auto bold_font_file = Configurations::get<std::string>("font.bold");
    auto italic_font_file = Configurations::get<std::string>("font.italic");
    if (!file_exists(font_file)) {
        throw std::runtime_error("Couldn't read font file " + font_file);
    }
    font = xd::create<xd::font>(font_file);
    if (!bold_font_file.empty()) {
        if (file_exists(bold_font_file)) {
            font->link_font("bold", xd::create<xd::font>(bold_font_file));
        } else {
            LOGGER_W << "Couldn't read bold font file " << bold_font_file;
        }
    }
    if (!italic_font_file.empty()) {
        if (file_exists(italic_font_file)) {
            font->link_font("italic", xd::create<xd::font>(italic_font_file));
        } else {
            LOGGER_W << "Couldn't read italic font file " << bold_font_file;
        }
    }

    auto clear_color = hex_to_color(Configurations::get<std::string>("startup.clear-color"));
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    if (editor_mode)
        return;
    map = Map::load(*this, Configurations::get<std::string>("startup.map"));
    // Create player object
    auto player_ptr = new Map_Object(
        *this,
        "player",
        Configurations::get<std::string>("startup.player-sprite"),
        xd::vec2(
            Configurations::get<float>("startup.player-position-x"),
            Configurations::get<float>("startup.player-position-y")
        )
    );
    player.reset(player_ptr);
    // Create input controller for player
    auto controller = xd::create<Player_Controller>(*this);
    player->add_component(controller);
    // Add player to the map
    map->add_object(player);
    // Play background music
    if (!map->get_bg_music_filename().empty()) {
        load_music(map->get_bg_music_filename());
        music->set_looping(true);
        music->play();
    }
    // Track player by camera
    camera->set_object(player_ptr);
    // Bind game keys
    process_keymap();
    // Setup Lua scripts
    pimpl->scripting_interface.reset(new Scripting_Interface(*this));
    pimpl->scripting_interface->set_globals();
    // Run game startup scripts
    std::string scripts_list =
        Configurations::get<std::string>("startup.scripts-list");
    if (!scripts_list.empty()) {
        std::ifstream scripts_file(normalize_slashes(scripts_list));
        if (scripts_file) {
            std::string filename;
            while (std::getline(scripts_file, filename)) {
                run_script(read_file(filename));
            }
        } else {
            throw std::runtime_error("Couldn't read file " + scripts_list);
        }
    }
    // Run map startup scripts
    map->run_startup_scripts();
    // Set frame update function and frequency
    int logic_fps = Configurations::get<int>("debug.logic-fps");
    window->register_tick_handler(std::bind(&Game::frame_update, this), 1000 / logic_fps);
    // Setup shader, if any
    set_shader(Configurations::get<std::string>("game.vertex-shader"),
        Configurations::get<std::string>("game.fragment-shader"));
}

Game::~Game() {
    xd::audio::shutdown();
}

void Game::run() {
    while (!pimpl->exit_requested) {
        window->update();
        if (window->closed())
            break;
        render();
    }
}

void Game::frame_update() {
    xd::audio::update();
    // Pause or resume game if needed
    bool triggered_pause = triggered("pause");
    if (pimpl->paused) {
        if (triggered_pause)
            resume();
        else if (pimpl->focus_pause && window->focused()) {
            resume();
            pimpl->focus_pause = false;
        }
    } else {
        if (triggered_pause)
            pause();
        else if (pimpl->pause_unfocused && !window->focused()) {
            pause();
            pimpl->focus_pause = true;
        }
    }
    if (pimpl->paused)
        return;

    set_current_scripting_interface(pimpl->scripting_interface.get());
    pimpl->scripting_interface->update();
    camera->update();
    map->update();
    // Switch map if needed
    if (!pimpl->next_map.empty())
        load_map(pimpl->next_map);
}

void Game::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    geometry.projection().load(
        xd::ortho<float>(
            0, static_cast<float>(game_width), // left, right
            static_cast<float>(game_height), 0, // bottom, top
            -1, 1 // near, far
        )
    );
    geometry.model_view().identity();
    auto cam_pos = camera->get_position();
    geometry.model_view().translate(-cam_pos.x, -cam_pos.y, 0);
    camera->render();
    if (!pimpl->editor_mode) {
        // Draw shader, if any
        if (pimpl->current_shader)
            pimpl->render_shader(*this);
        // Draw FPS
        if (pimpl->show_fps) {
            text_renderer.render(font, pimpl->debug_style, 5, 230,
                "FPS: " + boost::lexical_cast<std::string>(fps()));
        }
        // Draw game time
        if (pimpl->show_time || pimpl->paused) {
            auto seconds = std::to_string(clock->seconds());
            text_renderer.render(font, pimpl->debug_style, 5, 220, seconds);
        }
        window->swap();
    }
}

void Game::pause() {
    pimpl->paused = true;
    pimpl->pause_start_time = window->ticks();
    pimpl->was_stopped = clock->stopped();
    clock->stop_time();
    if (music) {
        pimpl->music_was_paused = music->paused();
        if (!pimpl->music_was_paused)
            music->pause();
    }
    set_shader(Configurations::get<std::string>("game.pause-vertex-shader"),
        Configurations::get<std::string>("game.pause-fragment-shader"));
}

void Game::resume() {
    pimpl->paused = false;
    pimpl->total_paused_time += window->ticks() - pimpl->pause_start_time;
    if (!pimpl->was_stopped)
        clock->resume_time();
    if (music && !pimpl->music_was_paused)
        music->play();
    set_shader(Configurations::get<std::string>("game.vertex-shader"),
        Configurations::get<std::string>("game.fragment-shader"));
}

void Game::exit() {
    pimpl->exit_requested = true;
}

void Game::set_size(int width, int height) {
    camera->calculate_viewport(width, height);
    camera->update_viewport();
}

void Game::run_script(const std::string& script) {
    set_current_scripting_interface(pimpl->scripting_interface.get());
    pimpl->scripting_interface->run_script(script);
}

xd::lua::virtual_machine* Game::get_lua_vm() {
    return &pimpl->vm;
}

xd::music::ptr Game::load_music(const std::string& filename) {
    if (music)
        music->stop();
    music.reset(new xd::music(filename));
    return music;
}

void Game::set_next_map(const std::string& filename, float x, float y, Direction dir) {
    pimpl->next_map = filename;
    pimpl->next_position.x = x;
    pimpl->next_position.y = y - player->get_bounding_box().y;
    pimpl->next_direction = dir;
}

xd::asset_manager& Game::get_asset_manager() {
    return pimpl->asset_manager;
}

bool Game::stopped() const {
    return clock->stopped();
}

int Game::seconds() const {
    return clock->seconds();
}

int Game::ticks() const {
    if (!window)
        return editor_ticks;
    int stopped_time = pimpl->total_paused_time + (pimpl->paused ?
        window->ticks() - pimpl->pause_start_time : 0);
    return window->ticks() - stopped_time;
}

void Game::set_shader(const std::string& vertex, const std::string& fragment) {
    std::string vsrc, fsrc;
    if (!vertex.empty() && !fragment.empty()) {
        try {
            vsrc = read_file(vertex);
            fsrc = read_file(fragment);
        } catch (const std::runtime_error& err) {
            LOGGER_W << "Error loading shaders: " << err.what();
            vsrc = fsrc = "";
        }
    }
    if (!vsrc.empty()) {
        pimpl->current_shader = new Custom_Shader(vsrc, fsrc);
        pimpl->full_screen_batch.set_shader(pimpl->current_shader);
    } else {
        pimpl->current_shader = nullptr;
    }
}

void Game::save(const std::string& filename, Save_File& save_file) const {
    try {
        std::ofstream ofs(normalize_slashes(filename), std::ios::binary);
        if (!ofs) {
            throw std::runtime_error("Unable to open file for writing");
        }
        ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        ofs << save_file;
    } catch (const std::ios_base::failure& e) {
        LOGGER_E << "Error saving file " << filename << " - error code: " << e.code() << " - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error saving file " << filename << " - message: " << e.what();
    }
}

std::unique_ptr<Save_File> Game::load(const std::string& filename) {
    lua_State* state = pimpl->scripting_interface->lua_state();
    std::unique_ptr<Save_File> file(new Save_File(state, luabind::object()));
    try {
        std::ifstream ifs(normalize_slashes(filename), std::ios::binary);
        if (!ifs) {
            throw std::runtime_error("File doesn't exist or can't be opened");
        }
        ifs.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        ifs >> *file;
    } catch (const std::ios_base::failure& e) {
        LOGGER_E << "Error loading file " << filename << " - error code: " << e.code() << " - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error loading file " << filename << " - message: " << e.what();
    }
    return file;
}

void Game::load_map(const std::string& filename) {
    map = Map::load(*this, filename);
    if (!pimpl->editor_mode) {
        // Add player to the map
        player->set_id(-1);
        player->set_position(pimpl->next_position);
        player->face(pimpl->next_direction);
        map->add_object(player);
        camera->set_object(player.get());
        player->set_triggered_object(nullptr);
        player->set_collision_area(nullptr);
        // Play background music
        auto bg_music = map->get_bg_music_filename();
        auto playing_music = music ? music->get_filename() : "";
        if (!bg_music.empty() && bg_music != playing_music) {
            load_music(bg_music);
            music->set_looping(true);
            music->play();
        }
        // Run map startup scripts
        map->run_startup_scripts();
        pimpl->next_map = "";
    }
}

void Game::new_map(xd::ivec2 map_size, xd::ivec2 tile_size) {
    map.reset(new Map(*this));
    map->resize(map_size, tile_size);
}

void Game::add_canvas(std::shared_ptr<Canvas> canvas) {
    map->add_canvas(canvas);
}

void Game::Impl::render_shader(Game& game) {
    int w = game.width();
    int h = game.height();
    if (!full_screen_texture)
        full_screen_texture = xd::create<xd::texture>(w, h, nullptr,
            xd::vec4(0), GL_CLAMP, GL_CLAMP);
    full_screen_texture->copy_read_buffer(0, 0, w, h);
    full_screen_batch.clear();
    full_screen_batch.add(full_screen_texture, 0, 0);
    game.geometry.projection().push(
    xd::ortho<float>(
            0, static_cast<float>(w), // left, right
            0, static_cast<float>(h), // bottom, top
            -1, 1 // near, far
        )
    );
    game.geometry.model_view().push(xd::mat4());
    glViewport(0, 0, w, h);
    game.window->clear();
    full_screen_batch.draw(game.get_mvp(), game.ticks());
    game.camera->update_viewport();
    game.geometry.model_view().pop();
    game.geometry.projection().pop();
}

void Game::process_keymap() {
    bool gamepad_enabled = window->joystick_present(0) &&
        Configurations::get<bool>("controls.gamepad-enabled");
    std::string map_file = Configurations::get<std::string>("controls.mapping-file");
    std::ifstream input(map_file);
    if (input) {
        // Map key names to XD keys
        std::unordered_map<std::string, xd::key> key_names;
        key_names["LEFT"] = xd::KEY_LEFT;
        key_names["RIGHT"] = xd::KEY_RIGHT;
        key_names["UP"] = xd::KEY_UP;
        key_names["DOWN"] = xd::KEY_DOWN;
        key_names["ENTER"] = xd::KEY_ENTER;
        key_names["SPACE"] = xd::KEY_SPACE;
        key_names["ESC"] = xd::KEY_ESC;
        // Assumes ASCII layout
        for (int i = xd::KEY_A.code; i <= xd::KEY_Z.code; ++i) {
            key_names[std::string(1, i)] = xd::KEY(i);
        }
        for (int i = xd::KEY_0.code; i <= xd::KEY_9.code; ++i) {
            key_names[std::string(1, i)] = xd::KEY(i);
        }
        if (gamepad_enabled) {
            key_names["GAMEPAD-LEFT"] = xd::JOYSTICK_AXIS_LEFT;
            key_names["GAMEPAD-RIGHT"] = xd::JOYSTICK_AXIS_RIGHT;
            key_names["GAMEPAD-UP"] = xd::JOYSTICK_AXIS_UP;
            key_names["GAMEPAD-DOWN"] = xd::JOYSTICK_AXIS_DOWN;
            for (int i = xd::JOYSTICK_BUTTON_1.code; i <= xd::JOYSTICK_BUTTON_12.code; ++i) {
                key_names["GAMEPAD-" + std::to_string(i + 1)] = xd::JOYSTICK(i);
            }
        }
        // Read keymap file and bind keys based on name
        std::string line;
        int counter = 0;
        while(std::getline(input, line))
        {
            ++counter;
            line = trim(line);
            if (line.empty() || line[0] == '#')
                continue;
            auto parts = split(line, "=");
            if (parts.size() < 2) {
                LOGGER_W << "Error processing key mapping file \"" << map_file <<
                    " at line " << counter << ", missing = sign.";
                continue;
            }
            std::string name = trim(parts[0]);
            auto keys = split(parts[1], ",");
            if (keys.empty())
                LOGGER_W << "Error processing key mapping file \"" << map_file <<
                    " at line " << counter << ", no keys specified.";
            for (auto& key : keys) {
                key = capitalize(trim(key));
                if (key_names.find(key) != key_names.end()) {
                    window->bind_key(key_names[key], name);
                } else if (gamepad_enabled || key.find("GAMEPAD") == std::string::npos) {
                    LOGGER_W << "Error processing key mapping file \"" << map_file <<
                    " at line " << counter << ", key \"" << key << "\" not found." ;
                    continue;
                }
            }
        }
    } else {
        // Default mapping
        LOGGER_W << "Couldn't read key mapping file \"" << map_file << "\", using default key mapping.";
        window->bind_key(xd::KEY_ESC, "pause");
        window->bind_key(xd::KEY_LEFT, "left");
        window->bind_key(xd::KEY_A, "left");
        window->bind_key(xd::KEY_RIGHT, "right");
        window->bind_key(xd::KEY_D, "right");
        window->bind_key(xd::KEY_UP, "up");
        window->bind_key(xd::KEY_W, "up");
        window->bind_key(xd::KEY_DOWN, "down");
        window->bind_key(xd::KEY_S, "down");
        window->bind_key(xd::KEY_ENTER, "a");
        window->bind_key(xd::KEY_SPACE, "a");
        window->bind_key(xd::KEY_Z, "a");
        window->bind_key(xd::KEY_J, "a");
        window->bind_key(xd::KEY_X, "b");
        window->bind_key(xd::KEY_K, "b");
        window->bind_key(xd::KEY_C, "c");
        window->bind_key(xd::KEY_L, "c");
        window->bind_key(xd::KEY_V, "d");
        window->bind_key(xd::KEY_I, "d");
        if (gamepad_enabled) {
            window->bind_key(xd::JOYSTICK_AXIS_UP, "up");
            window->bind_key(xd::JOYSTICK_AXIS_DOWN, "down");
            window->bind_key(xd::JOYSTICK_AXIS_LEFT, "left");
            window->bind_key(xd::JOYSTICK_AXIS_RIGHT, "right");
            window->bind_key(xd::JOYSTICK_BUTTON_1, "a");
            window->bind_key(xd::JOYSTICK_BUTTON_2, "b");
            window->bind_key(xd::JOYSTICK_BUTTON_3, "c");
            window->bind_key(xd::JOYSTICK_BUTTON_4, "d");
            window->bind_key(xd::JOYSTICK_BUTTON_8, "pause");
        }
    }
}
