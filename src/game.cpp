#include "../include/game.hpp"
#include "../include/audio_player.hpp"
#include "../include/clock.hpp"
#include "../include/player_controller.hpp"
#include "../include/camera.hpp"
#include "../include/object_layer.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/scripting/scripting_interface.hpp"
#include "../include/configurations.hpp"
#include "../include/utility/color.hpp"
#include "../include/utility/direction.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/string.hpp"
#include "../include/save_file.hpp"
#include "../include/decorators/shake_decorator.hpp"
#include "../include/decorators/typewriter_decorator.hpp"
#include "../include/key_binder.hpp"
#include "../include/log.hpp"
#include "../include/exceptions.hpp"
#include "../include/vendor/platform_folders.hpp"
#include "../include/xd/audio.hpp"
#include "../include/xd/graphics/font.hpp"
#include "../include/xd/graphics/stock_text_formatter.hpp"
#include "../include/xd/asset_manager.hpp"
#include "../include/xd/lua/virtual_machine.hpp"
#include "../include/xd/vendor/sol/sol.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <unordered_set>

struct Game::Impl {
    explicit Impl(Game& game, std::shared_ptr<xd::audio> audio, bool editor_mode) :
            audio_player(audio),
            editor_mode(editor_mode),
            show_fps(Configurations::get<bool>("debug.show-fps")),
            show_time(Configurations::get<bool>("debug.show-time")),
            next_direction(Direction::DOWN),
            focus_pause(false),
            pause_unfocused(Configurations::get<bool>("game.pause-unfocused")),
            was_stopped(false),
            pause_start_time(0),
            total_paused_time(0),
            exit_requested(false),
            fullscreen_change_ticks(-1),
            fullscreen_update_delay(1),
            was_fullscreen(false),
            debug_style(xd::vec4(1.0f), Configurations::get<int>("font.size")),
            game_width(Configurations::get<int>("graphics.game-width", "debug.width")),
            game_height(Configurations::get<int>("graphics.game-height", "debug.height")),
            gamepad_id(Configurations::get<int>("controls.gamepad-number")),
            preferred_gamepad_id(Configurations::get<int>("controls.gamepad-number")),
            pause_button(Configurations::get<std::string>("controls.pause-button")),
            scripts_folder(Configurations::get<std::string>("game.scripts-folder")),
            reset_scripting(false),
            text_formatter(
                make_icon_texture(),
                xd::vec2{Configurations::get<float>("font.icon-width"), Configurations::get<float>("font.icon-height")},
                xd::vec2{Configurations::get<float>("font.icon-offset-x"), Configurations::get<float>("font.icon-offset-y")}),
            shake_decorator(game),
            typewriter_decorator(game, audio_player) {

        // Register decorators
        text_formatter.register_decorator("shake", [=](xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args) {
            shake_decorator(decorator, text, args);
        });
        text_formatter.register_decorator("typewriter", [=](xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args) {
            typewriter_decorator(decorator, text, args);
        });

        // Scripts folder
        if (!scripts_folder.empty() && scripts_folder.back() != '/') {
            scripts_folder += '/';
        }

        // Script preamble
        auto preamble = string_utilities::trim(Configurations::get<std::string>("game.object-script-preamble"));
        if (preamble.empty()) return;

        auto extension = preamble.substr(preamble.find_last_of(".") + 1);
        if (extension == "lua") {
            auto fs = file_utilities::game_data_filesystem();
            preamble = fs->read_file(scripts_folder + preamble);
        }

        object_script_preamble = preamble + ";";
    }
    // Create icon texture
    static std::shared_ptr<xd::texture> make_icon_texture() {
        auto filename = Configurations::get<std::string>("font.icon-image");
        if (filename.empty()) return nullptr;

        auto fs = file_utilities::game_data_filesystem();
        auto stream = fs->open_binary_ifstream(filename);
        if (!stream || !*stream) {
            throw file_loading_exception{ "Failed to load icon texture " + filename };
        }

        return std::make_shared<xd::texture>(filename, *stream,
            hex_to_color(Configurations::get<std::string>("font.icon-transparent-color"))
        );
    }
    // Called when a configuration changes
    void on_config_change(const std::string& config_key) {
        config_changes.insert(config_key);
    }
    bool config_changed(const std::string& config_key) const {
        return config_changes.find(config_key) != config_changes.end();
    }
    // Process configuration changes
    void process_config_changes(Game& game, xd::window* window) {
        if (config_changes.empty()) return;
        if (config_changed("game.pause-unfocused")) {
            pause_unfocused = Configurations::get<bool>("game.pause-unfocused");
        }
        if (game.is_fullscreen() && (config_changed("graphics.screen-width") || config_changed("graphics.screen-height"))) {
            game.set_size(Configurations::get<int>("graphics.screen-width"),
                Configurations::get<int>("graphics.screen-height"));
        }
        if (!game.is_fullscreen() && (config_changed("graphics.window-width") || config_changed("graphics.window-height"))) {
            game.set_size(Configurations::get<int>("graphics.window-width"),
                Configurations::get<int>("graphics.window-height"));
        }
        if (config_changed("graphics.fullscreen")) {
            start_fullscreen_change(game);
        }
        if (config_changed("graphics.vsync") && window) {
            window->set_vsync(Configurations::get<bool>("graphics.vsync"));
        }
        if (config_changed("graphics.scale-mode")) {
            game.get_camera()->set_size(game.framebuffer_width(), game.framebuffer_height(), true);
        }
        if (config_changed("graphics.brightness")) {
            game.get_camera()->set_brightness(Configurations::get<float>("graphics.brightness"));
        }
        if (config_changed("graphics.contrast")) {
            game.get_camera()->set_contrast(Configurations::get<float>("graphics.contrast"));
        }
        if (config_changed("graphics.gamma") && window) {
            window->set_gamma(Configurations::get<float>("graphics.gamma"));
        }
        if (config_changed("audio.music-volume")) {
            audio_player.set_global_music_volume(Configurations::get<float>("audio.music-volume"));
        }
        if (config_changed("audio.sound-volume")) {
            audio_player.set_global_sound_volume(Configurations::get<float>("audio.sound-volume"));
        }
        if (config_changed("controls.gamepad-enabled") && window) {
            window->set_joystick_enabled(Configurations::get<bool>("controls.gamepad-enabled"));
        }
        if (config_changed("controls.gamepad-number") && window) {
            gamepad_id = -1;
            preferred_gamepad_id = Configurations::get<int>("controls.gamepad-number");
            get_gamepad_id(*window);
        }
        if (config_changed("debug.show-fps")) {
            show_fps = Configurations::get<bool>("debug.show-fps");
        }
        config_changes.clear();
    }
    // Process key-mapping string
    void process_keymap(Game& game) {
        if (!key_binder) {
            key_binder = std::make_unique<Key_Binder>(game);
        }

        auto user_data_folder = file_utilities::user_data_folder();
        if (!user_data_folder->load_keymap_file(*key_binder)) {
            key_binder->bind_defaults();
        }
    }
    // Get preferred gamepad ID (or first one)
    int get_gamepad_id(xd::window& window) {
        if (gamepad_id == -1 || !window.joystick_present(gamepad_id)) {
            if (gamepad_id != preferred_gamepad_id
                    && preferred_gamepad_id != -1
                    && window.joystick_present(preferred_gamepad_id)) {
                gamepad_id = preferred_gamepad_id;
            } else {
                gamepad_id = window.first_joystick_id();
            }
        }
        return gamepad_id;
    }
    void reset_scripting_interface(Game& game) {
        scripting_interface = std::make_unique<Scripting_Interface>(game);
        scripting_interface->set_globals();
        // Run game startup scripts
        LOGGER_I << "Running game startup scripts";
        std::string scripts_list =
            Configurations::get<std::string>("startup.scripts-list");

        if (scripts_list.empty()) {
            reset_scripting = false;
            return;
        }

        auto fs = file_utilities::game_data_filesystem();
        auto scripts_file = fs->open_ifstream(scripts_list);
        if (!scripts_file || !*scripts_file) {
            throw std::runtime_error("Couldn't read file " + scripts_list);
        }

        std::string filename;
        while (std::getline(*scripts_file, filename)) {
            if (string_utilities::ends_with(filename, "\r"))
                filename = filename.substr(0, filename.size() - 1);
            run_script(game, filename, true);
        }

        reset_scripting = false;
    }
    void run_script(Game& game, const std::string& script_or_filename, bool is_filename) {
        auto& si = game.is_paused() ? pause_scripting_interface : scripting_interface;
        auto old_interface = game.get_current_scripting_interface();
        game.set_current_scripting_interface(si.get());
        if (is_filename) {
            si->schedule_file(scripts_folder + script_or_filename, "GLOBAL");
        } else {
            si->schedule_code(script_or_filename, "GLOBAL");
        }
        game.set_current_scripting_interface(old_interface);
    }
    void run_function(Game& game, const sol::protected_function& function) {
        auto& si = game.is_paused() ? pause_scripting_interface : scripting_interface;
        auto old_interface = game.get_current_scripting_interface();
        game.set_current_scripting_interface(si.get());
        si->schedule_function(function, "GLOBAL");
        game.set_current_scripting_interface(old_interface);
    }
    void start_fullscreen_change(Game& game) {
        if (fullscreen_change_ticks != -1) {
            end_fullscreen_change(game);
        }
        fullscreen_change_ticks = game.window_ticks();
        was_fullscreen = game.is_fullscreen();
        game.set_fullscreen(Configurations::get<bool>("graphics.fullscreen"));
    }
    void end_fullscreen_change(Game& game) {
        // Need to wait in some operating systems for the mode change to apply
        if (game.window_ticks() - fullscreen_change_ticks < fullscreen_update_delay) {
            return;
        }

        fullscreen_change_ticks = -1;

        auto is_fullscreen = game.is_fullscreen();
        if (is_fullscreen && !was_fullscreen) {
            game.set_size(Configurations::get<int>("graphics.screen-width"),
                Configurations::get<int>("graphics.screen-height"));
        } else if (!is_fullscreen && was_fullscreen) {
            game.set_size(Configurations::get<int>("graphics.window-width"),
                Configurations::get<int>("graphics.window-height"));
        }
    }
    // Audio subsystem
    Audio_Player audio_player;
    // Was game started in editor mode?
    bool editor_mode;
    // Texture asset manager
    xd::asset_manager asset_manager;
    // The shared Lua virtual machine
    xd::lua::virtual_machine vm;
    // Game-specific scripting interface
    std::unique_ptr<Scripting_Interface> scripting_interface;
    // Scripting interface used when the game is paused
    std::unique_ptr<Scripting_Interface> pause_scripting_interface;
    // Show frames per second?
    bool show_fps;
    // Show current game-time in seconds?
    bool show_time;
    // Information for teleporting to another map
    std::optional<xd::vec2> next_position;
    Direction next_direction;
    std::string next_map;
    std::optional<std::string> next_music;
    // Was it paused because screen got unfocused?
    bool focus_pause;
    // Should the game be paused while unfocused?
    bool pause_unfocused;
    // Was time stopped when game got paused?
    bool was_stopped;
    // Keep track of paused time
    int pause_start_time;
    int total_paused_time;
    // Information about a fullscreen change
    int fullscreen_change_ticks;
    int fullscreen_update_delay;
    bool was_fullscreen;
    // Is it time to exit the main loop?
    bool exit_requested;
    // Debug font style (FPS and time display)
    xd::font_style debug_style;
    // Game width
    int game_width;
    // Game height
    int game_height;
    // Active gamepad id
    int gamepad_id;
    // Configured gamepad ID
    int preferred_gamepad_id;
    // Unapplied config changes
    std::unordered_set<std::string> config_changes;
    // Keymap file reader and binder
    std::unique_ptr<Key_Binder> key_binder;
    // The button for pausing the game
    std::string pause_button;
    // Base folder for startup and map scripts
    std::string scripts_folder;
    // Should the scripting interface be reset?
    bool reset_scripting;
    // String added before every map object script
    std::string object_script_preamble;
    // Text formatter
    xd::stock_text_formatter text_formatter;
    // Text renderer
    xd::simple_text_renderer text_renderer;
    // {shake} decorator
    Shake_Decorator shake_decorator;
    // {typewriter} decorator
    Typewriter_Decorator typewriter_decorator;
    // Font cache
    std::unordered_map<std::string, std::shared_ptr<xd::font>> fonts;
};

Game::Game(const std::vector<std::string>& args, std::shared_ptr<xd::audio> audio, bool editor_mode) :
        command_line_args(args),
        window(editor_mode ? nullptr : std::make_unique<xd::window>(
            Configurations::get<std::string>("game.title"),
            Configurations::get<bool>("graphics.fullscreen")
                ? Configurations::get<int>("graphics.screen-width")
                : Configurations::get<int>("graphics.window-width"),
            Configurations::get<bool>("graphics.fullscreen")
                ? Configurations::get<int>("graphics.screen-height")
                : Configurations::get<int>("graphics.window-height"),
            xd::window_options(
                Configurations::get<bool>("graphics.fullscreen"),
                Configurations::get<int>("graphics.game-width", "debug.width"),
                Configurations::get<int>("graphics.game-height", "debug.height"),
                0.8f, // max windowed size percentage
                false, // allow resize
                false, // display cursor
                Configurations::get<bool>("graphics.vsync"),
                Configurations::get<bool>("controls.gamepad-enabled"),
                Configurations::get<bool>("controls.gamepad-detection"),
                Configurations::get<bool>("controls.axis-as-dpad"),
                Configurations::get<float>("controls.stick-sensitivity"),
                Configurations::get<float>("controls.trigger-sensitivity"),
                8, // depth
                0, // stencil
                0, // antialiasing
                2, // GL major version
                0))), // GL minor version
        pimpl(std::make_unique<Impl>(*this, audio, editor_mode)),
        paused(false),
        pausing_enabled(true),
        magnification(Configurations::get<float>("graphics.magnification", "debug.magnification")),
        current_scripting_interface(nullptr),
        style(xd::vec4(1.0f, 1.0f, 1.0f, 1.0f), Configurations::get<int>("font.size")),
        editor_ticks(0),
        editor_size(1, 1) {
    // Set members
    clock = std::make_unique<Clock>(*this);
    camera = std::make_unique<Camera>(*this);
    framebuffer = std::make_unique<xd::framebuffer>();

    // Listen to config changes
    Configurations::add_observer("Game",
        [this](const std::string& key) {
            this->pimpl->on_config_change(key);
        }
    );

    // Setup fonts
    LOGGER_I << "Setting up fonts";
    style.outline(1, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f))
        .line_height(Configurations::get<float>("font.line-height"))
        .force_autohint(true);
    pimpl->debug_style.line_height(12.0f).force_autohint(true);
    auto font_file = Configurations::get<std::string>("font.default");


    auto fs = file_utilities::game_data_filesystem();
    if (!fs->file_exists(font_file)) {
        throw std::runtime_error("Couldn't read font file " + font_file);
    }
    font = create_font(font_file);

    auto bold_font_file = Configurations::get<std::string>("font.bold");
    if (!bold_font_file.empty()) {
        if (!fs->file_exists(bold_font_file)) {
            throw std::runtime_error("Couldn't read bold font file " + bold_font_file);
        }
        font->link_font("bold", create_font(bold_font_file));
    }

    auto italic_font_file = Configurations::get<std::string>("font.italic");
    if (!italic_font_file.empty()) {
        if (!fs->file_exists(italic_font_file)) {
            throw std::runtime_error("Couldn't read bold font file " + italic_font_file);
        }
        font->link_font("italic", create_font(italic_font_file));
    }

    if (editor_mode)
        return;

    window->set_gamma(Configurations::get<float>("graphics.gamma"));

    auto startup_map = args.size() > 1 && string_utilities::ends_with(args[1], ".tmx")
        ? args[1]
        : Configurations::get<std::string>("startup.map");
    map = Map::load(*this, startup_map);

    auto start_pos = map->get_starting_position();
    if (Configurations::has_value("startup.player-position-x")) {
        start_pos.x = Configurations::get<float>("startup.player-position-x");
    }
    if (Configurations::has_value("startup.player-position-y")) {
        start_pos.y = Configurations::get<float>("startup.player-position-y");
    }

    LOGGER_I << "Creating player object";
    // Create player object
    player = std::make_shared<Map_Object>(
        *this,
        "player",
        Configurations::get<std::string>("startup.player-sprite"),
        start_pos);
    // Create input controller for player
    auto controller = std::make_shared<Player_Controller>(*this);
    player->add_component(controller);
    // Add player to the map
    map->add_object(player);
    // Play background music and ambient
    pimpl->audio_player.play_music(*map);
    pimpl->audio_player.play_ambient(*map);
    // Track player by camera
    camera->set_object(player.get());
    // Bind game keys
    pimpl->process_keymap(*this);
    // Setup Lua scripts
    pimpl->reset_scripting_interface(*this);
    // Script run when game is paused
    auto pause_script = Configurations::get<std::string>("game.pause-script");
    if (!pause_script.empty()) {
        pimpl->pause_scripting_interface = std::make_unique<Scripting_Interface>(*this);
        pimpl->pause_scripting_interface->set_globals();
    }
    // Run map startup scripts
    map->run_startup_scripts();
    // Set frame update function and frequency
    int logic_fps = Configurations::get<int>("graphics.logic-fps", "debug.logic-fps");
    window->register_tick_handler(std::bind(&Game::frame_update, this), 1000 / logic_fps);
    // Log errors
    window->register_error_handler([](int code, const char* description) {
        LOGGER_E << "GLFW error (" << code << "): " << description;
    });
    // Setup shader, if any
    camera->set_shader(Configurations::get<std::string>("graphics.vertex-shader"),
        Configurations::get<std::string>("graphics.fragment-shader"));
    camera->update();
}

Game::~Game() {
    Configurations::remove_observer("Game");
}

channel_group_type Game::get_sound_group_type() const {
    return pimpl->audio_player.get_sound_group_type(!paused);
}

void Game::run() {
    while (!pimpl->exit_requested) {
        window->update();
        if (window->closed())
            break;
        render();
    }

    file_utilities::user_data_folder()->try_to_save_config();
}

void Game::frame_update() {
    pimpl->audio_player.update();

    // Toggle fullscreen when ALT+Enter is pressed
    if ((pressed(xd::KEY_RALT) || pressed(xd::KEY_LALT)) && triggered_once(xd::KEY_ENTER)) {
        Configurations::set("graphics.fullscreen", !is_fullscreen());
    }

    // Pause or resume game if needed
    bool triggered_pause = triggered(pimpl->pause_button);
    // Only resume if there is no pause script, otherwise the script should do it
    if (paused && !pimpl->pause_scripting_interface) {
        if (triggered_pause)
            resume();
        else if (pimpl->focus_pause && window->focused()) {
            resume();
            pimpl->focus_pause = false;
        }
    } else if (!paused && pausing_enabled) {
        if (triggered_pause)
            pause();
        else if (pimpl->pause_unfocused && !window->focused()) {
            pause();
            pimpl->focus_pause = true;
        }
    }

    if (paused) {
        if (pimpl->pause_scripting_interface) {
            set_current_scripting_interface(pimpl->pause_scripting_interface.get());
            pimpl->pause_scripting_interface->update();
            if (pimpl->reset_scripting) {
                pimpl->reset_scripting_interface(*this);
            }
        }

        // We still update map canvases, but not scripts
        if (pimpl->next_map.empty())
            map->update();
        pimpl->process_config_changes(*this, window.get());
        return;
    }

    set_current_scripting_interface(pimpl->scripting_interface.get());
    pimpl->scripting_interface->update();

    if (pimpl->reset_scripting) {
        pimpl->reset_scripting_interface(*this);
    }

    camera->update();

    if (pimpl->next_map.empty()) {
        map->update();
    }

    if (pimpl->fullscreen_change_ticks != -1) {
        pimpl->end_fullscreen_change(*this);
    }

    pimpl->process_config_changes(*this, window.get());

    // Switch map if needed
    if (!pimpl->next_map.empty()) {
        load_next_map();
    }
}

void Game::render() {
    // Window size changes are asynchronous in X11 so we keep polling the size
    camera->set_size(framebuffer_width(), framebuffer_height());
    camera->render();

    if (pimpl->editor_mode) return;

    // Draw FPS
    if (pimpl->show_fps) {
        pimpl->text_renderer.render(*font, pimpl->debug_style,
            camera->get_geometry().projection().get(), 5, 10,
            "FPS: " + std::to_string(fps()));
    }

    // Draw game time
    if (pimpl->show_time) {
        auto seconds = std::to_string(clock->seconds());
        pimpl->text_renderer.render(*font, pimpl->debug_style,
            camera->get_geometry().projection().get(), 5, 20, seconds);
    }

    window->swap();
}

bool Game::in_editor_mode() const {
    return pimpl->editor_mode;
}

void Game::pause() {
    paused = true;
    pimpl->pause_start_time = window->ticks();
    pimpl->was_stopped = clock->stopped();
    clock->stop_time();

    pimpl->audio_player.pause();

    camera->set_shader(Configurations::get<std::string>("graphics.pause-vertex-shader"),
        Configurations::get<std::string>("graphics.pause-fragment-shader"));

    if (pimpl->pause_scripting_interface) {
        auto pause_script = Configurations::get<std::string>("game.pause-script");
        run_script_file(pause_script);
    }
}

void Game::resume(const std::string& script) {
    paused = false;
    pimpl->total_paused_time += window->ticks() - pimpl->pause_start_time;

    if (!pimpl->was_stopped) clock->resume_time();

    pimpl->audio_player.resume();

    camera->set_shader(Configurations::get<std::string>("graphics.vertex-shader"),
        Configurations::get<std::string>("graphics.fragment-shader"));

    if (!script.empty()) run_script(script);
}

void Game::exit() {
    pimpl->exit_requested = true;
}

void Game::set_size(int width, int height) {
    LOGGER_I << "Setting screen size to " << width << ", " << height;
    if (pimpl->editor_mode) {
        editor_size = xd::ivec2(width, height);
        pimpl->game_width = static_cast<int>(map->get_pixel_width() * magnification);
        pimpl->game_height = static_cast<int>(map->get_pixel_height() * magnification);
    } else {
        window->set_size(width, height);
    }
}

xd::vec2 Game::get_monitor_size() const {
    return window ? window->get_size() : xd::vec2{0.0f, 0.0f};
}

std::vector<xd::vec2> Game::get_sizes() const {
    return window ? window->get_sizes() : std::vector<xd::vec2>{};
}

void Game::set_fullscreen(bool fullscreen) {
    if (!window || fullscreen == is_fullscreen()) return;
    LOGGER_I << "Changing window display to " << (fullscreen ? "fullscreen" : "windowed");
    window->set_fullscreen(fullscreen);
    camera->set_size(framebuffer_width(), framebuffer_height());
}

int Game::game_width(bool magnified) const {
    return magnified
        ? static_cast<int>(pimpl->game_width / magnification)
        : pimpl->game_width;
}

int Game::game_height(bool magnified) const {
    return magnified
        ? static_cast<int>(pimpl->game_height / magnification)
        : pimpl->game_height;
}

void Game::set_magnification(float mag) {
    magnification = mag;
    camera->calculate_viewport(framebuffer_width(), framebuffer_height());
    camera->update_viewport();
}

std::vector<std::string> Game::triggered_keys() const {
    std::vector<std::string> results;
    auto keys = window->triggered_keys();
    for (xd::key key : keys) {
        if (key.type == xd::input_type::INPUT_GAMEPAD && key.device_id != get_gamepad_id()) continue;
        key.device_id = -1;
        results.push_back(pimpl->key_binder->get_key_identifier(key));
    }
    return results;
}

void Game::bind_key(const std::string& physical_name, const std::string& virtual_name) {
    pimpl->key_binder->bind_key(physical_name, virtual_name);
}

void Game::unbind_physical_key(const std::string& physical_name) {
    pimpl->key_binder->unbind_key(physical_name);
}

void Game::unbind_virtual_key(const std::string& virtual_name) {
    window->unbind_key(virtual_name);
    pimpl->key_binder->remove_virtual_name(virtual_name);
}

std::vector<std::string> Game::get_bound_keys(const std::string& virtual_name) const {
    return pimpl->key_binder->get_bound_keys(virtual_name);
}


std::string Game::get_key_name(const std::string& physical_key) const {
    auto keys = pimpl->key_binder->get_keys(physical_key);
    if (keys.empty()) return "";
    return window->key_name(keys.front());
}

void Game::run_script(const std::string& script) {
    pimpl->run_script(*this, script, false);
}

void Game::run_script_file(const std::string& filename) {
    pimpl->run_script(*this, filename, true);
}

void Game::run_function(const sol::protected_function& function) {
    pimpl->run_function(*this, function);
}

xd::lua::virtual_machine* Game::get_lua_vm() {
    return &pimpl->vm;
}

void Game::reset_scripting() {
    pimpl->scripting_interface->get_scheduler().pause();
    pimpl->reset_scripting = true;
}

bool Game::is_script_scheduler_paused() const {
    return pimpl->scripting_interface->get_scheduler().paused();
}

void Game::set_script_scheduler_paused(bool paused) {
    auto& scheduler = pimpl->scripting_interface->get_scheduler();
    if (paused)
        scheduler.pause();
    else
        scheduler.resume();
}

Audio_Player& Game::get_audio_player() {
    return pimpl->audio_player;
}

void Game::set_next_map(const std::string& filename, Direction dir, std::optional<xd::vec2> pos, std::optional<std::string> music) {
    pimpl->next_map = filename;
    pimpl->next_direction = dir == Direction::NONE ? player->get_direction() : dir;
    pimpl->next_position = pos;
    pimpl->next_music = music;
}

xd::asset_manager& Game::get_asset_manager() {
    return pimpl->asset_manager;
}

std::shared_ptr<xd::font> Game::create_font(const std::string& filename) {
    auto& fonts = pimpl->fonts;
    if (fonts.find(filename) != fonts.end()) return fonts[filename];

    auto fs = file_utilities::game_data_filesystem();
    if (!fs->file_exists(filename))
        throw std::runtime_error("Couldn't read font file " + filename);

    // The font face owns the filestream and loads the font as needed
    auto stream_ptr = fs->open_binary_ifstream(filename);
    fonts[filename] = std::make_shared<xd::font>(filename, std::move(stream_ptr));

    return fonts[filename];
}

void Game::render_text(xd::font& font, const xd::font_style& style, float x, float y, const std::string& text) {
    pimpl->text_renderer.render_formatted(font, pimpl->text_formatter,
        style, camera->get_geometry().projection().get(), x, y, text);
}

float Game::text_width(const std::string& text, xd::font* font, const xd::font_style* style) {
    return pimpl->text_renderer.text_width(font ? *font : *this->font, pimpl->text_formatter,
        style ? *style : this->style, text);
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
    int stopped_time = pimpl->total_paused_time + (paused ?
        window->ticks() - pimpl->pause_start_time : 0);
    return window->ticks() - stopped_time;
}

std::string Game::get_scripts_directory() const {
    return pimpl->scripts_folder;
}

void Game::load_next_map() {
    map = Map::load(*this, pimpl->next_map);
    if (pimpl->editor_mode) return;

    // Reset the player's references
    player->set_id(-1);
    player->set_triggered_object(nullptr);
    player->set_collision_object(nullptr);
    player->set_collision_area(nullptr);
    player->set_outlining_object(nullptr);
    player->clear_linked_objects();

    // Add the player to the map
    auto start_pos = pimpl->next_position.value_or(map->get_starting_position());
    player->set_position(start_pos);
    player->face(pimpl->next_direction);
    map->add_object(player);
    camera->set_object(player.get());
    camera->update();

    // Play background music and ambient
    auto& audio_player = pimpl->audio_player;
    audio_player.load_map_audio(*map);
    auto bg_music = pimpl->next_music.value_or(map->get_bg_music_filename());
    audio_player.play_music(*map, bg_music, true, map->get_bg_music_volume());
    audio_player.play_ambient(*map);

    map->run_startup_scripts();
    pimpl->next_map = "";
}

void Game::new_map(xd::ivec2 map_size, xd::ivec2 tile_size) {
    map = std::make_unique<Map>(*this);
    map->resize(map_size, tile_size);
    map->add_layer(Layer_Type::OBJECT);
}

void Game::add_canvas(std::shared_ptr<Base_Canvas> canvas) {
    map->add_canvas(canvas);
}

bool Game::gamepad_enabled() const {
    int gamepad_id = get_gamepad_id();
    return Configurations::get<bool>("controls.gamepad-enabled") && window->joystick_present(gamepad_id);
}

int Game::get_gamepad_id() const {
    if (!window) return -1;
    return pimpl->get_gamepad_id(*window);
}

std::optional<std::string> Game::get_gamepad_name() const {
    std::optional<std::string> current_name = std::nullopt;
    if (!window || !gamepad_enabled()) return current_name;

    auto current_id = get_gamepad_id();
    if (current_id == -1) {
        current_name = window->first_joystick_name();
    }  else {
        auto names = window->joystick_names();
        for (auto& [id, name] : names) {
            if (current_id == id) {
                current_name = name;
                break;
            }
        }
    }
    return current_name;
}

std::string Game::get_object_script_preamble() const {
    return pimpl->object_script_preamble;
}

Typewriter_Decorator& Game::get_typewriter_decorator() {
    return pimpl->typewriter_decorator;
}

Key_Binder& Game::get_key_binder() {
    return *pimpl->key_binder;
}
