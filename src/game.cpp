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
#include "../include/utility/color.hpp"
#include "../include/utility/direction.hpp"
#include "../include/utility/file.hpp"
#include "../include/save_file.hpp"
#include "../include/shake_decorator.hpp"
#include "../include/key_binder.hpp"
#include "../include/log.hpp"
#include "../include/vendor/platform_folders.hpp"
#include "../include/xd/audio.hpp"
#include "../include/xd/graphics.hpp"
#include "../include/xd/asset_manager.hpp"
#include "../include/xd/lua/virtual_machine.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <unordered_set>

struct Game::Impl {
    explicit Impl(xd::audio* audio, bool editor_mode) :
            audio(audio),
            editor_mode(editor_mode),
            show_fps(Configurations::get<bool>("debug.show-fps")),
            show_time(Configurations::get<bool>("debug.show-time")),
            next_direction(Direction::DOWN),
            focus_pause(false),
            music_was_paused(false),
            was_stopped(false),
            pause_start_time(0),
            total_paused_time(0),
            exit_requested(false),
            debug_style(xd::vec4(1.0f), Configurations::get<int>("font.size")),
            game_width(Configurations::get<float>("debug.width")),
            game_height(Configurations::get<float>("debug.height")),
            gamepad_id(Configurations::get<int>("controls.gamepad-number")),
            save_path("not set"),
            pause_button(Configurations::get<std::string>("controls.pause-button")),
            reset_scripting(false) {}
    // Called when a configuration changes
    void on_config_change(const std::string& config_key) {
        config_changes.insert(config_key);
    }
    bool config_changed(const std::string& config_key) const {
        return config_changes.find(config_key) != config_changes.end();
    }
    // Process configuration changes
    void process_config_changes(Game& game, xd::window* window) {
        if (config_changed("graphics.screen-width") || config_changed("graphics.screen-height")) {
            game.set_size(Configurations::get<int>("graphics.screen-width"),
                Configurations::get<int>("graphics.screen-height"));
        }
        if (config_changed("graphics.fullscreen")) {
            game.set_fullscreen(Configurations::get<bool>("graphics.fullscreen"));
        }
        if (config_changed("graphics.vsync") && window) {
            window->set_vsync(Configurations::get<bool>("graphics.vsync"));
        }
        if (config_changed("graphics.scale-mode")) {
            game.get_camera()->set_size(game.width(), game.height(), true);
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
            game.set_global_music_volume(Configurations::get<float>("audio.music-volume"));
        }
        if (config_changed("audio.sound-volume")) {
            game.set_global_sound_volume(Configurations::get<float>("audio.sound-volume"));
        }
        if (config_changed("controls.gamepad-enabled") && window) {
            window->set_joystick_enabled(Configurations::get<bool>("controls.gamepad-enabled"));
        }
        if (config_changed("controls.gamepad-number") && window) {
            gamepad_id = -1;
            get_gamepad_id(*window);
        }
        if (config_changed("debug.show-fps")) {
            show_fps = Configurations::get<bool>("debug.show-fps");
        }
        config_changes.clear();
    }
    // Get default save folder
    std::string get_save_directory() {
        if (!save_path.empty() && (save_path == "not set" || !file_utilities::file_exists(save_path))) {
            save_path = file_utilities::get_data_directory();
        }
        return save_path;
    }
    // Add directory to save filename
    void cleanup_save_filename(std::string& filename) {
        file_utilities::normalize_slashes(filename);
        if (filename.find('/') == std::string::npos) {
            filename = get_save_directory() + filename;
        }
    }
    // Process key-mapping string
    void process_keymap(Game& game, xd::window* window, bool gamepad_only = false) {
        if (!key_binder) {
            key_binder = std::make_unique<Key_Binder>(game);
        }
        if (!key_binder->process_keymap_file()) {
            key_binder->bind_defaults();
        }
    }
    // Get preferred gamepad ID (or first one)
    int get_gamepad_id(xd::window& window) {
        if (gamepad_id == -1 || !window.joystick_present(gamepad_id)) {
            int preferred_id = Configurations::get<int>("controls.gamepad-number");
            if (gamepad_id != preferred_id && preferred_id != -1 && window.joystick_present(preferred_id)) {
                gamepad_id = preferred_id;
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
        file_utilities::normalize_slashes(scripts_list);
        if (!scripts_list.empty()) {
            std::ifstream scripts_file(scripts_list);
            if (scripts_file) {
                std::string filename;
                while (std::getline(scripts_file, filename)) {
                    if (string_utilities::ends_with(filename, "\r"))
                        filename = filename.substr(0, filename.size() - 1);
                    game.run_script(file_utilities::read_file(filename));
                }
            } else {
                throw std::runtime_error("Couldn't read file " + scripts_list);
            }
        }
        reset_scripting = false;
    }
    // Audio system
    xd::audio* audio;
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
    // Was it paused because screen got unfocused?
    bool focus_pause;
    // was music already paused when game got paused?
    bool music_was_paused;
    // Was time stopped when game got paused?
    bool was_stopped;
    // Keep track of paused time
    int pause_start_time;
    int total_paused_time;
    // Is it time to exit the main loop?
    bool exit_requested;
    // Debug font style (FPS and time display)
    xd::font_style debug_style;
    // Game width
    float game_width;
    // Game height
    float game_height;
    // Active gamepad id
    int gamepad_id;
    // Unapplied config changes
    std::unordered_set<std::string> config_changes;
    // Calculated save folder path
    std::string save_path;
    // Keymap file reader and binder
    std::unique_ptr<Key_Binder> key_binder;
    // The button for pausing the game
    std::string pause_button;
    // Should the scripting interface be reset?
    bool reset_scripting;
};

Game::Game(xd::audio* audio, bool editor_mode) :
        pimpl(std::make_unique<Impl>(audio, editor_mode)),
        window(editor_mode ? nullptr : std::make_unique<xd::window>(
            Configurations::get<std::string>("game.title"),
            Configurations::get<int>("graphics.screen-width"),
            Configurations::get<int>("graphics.screen-height"),
            xd::window_options(
                Configurations::get<bool>("graphics.fullscreen"),
                false, // allow resize
                false, // display_cursor
                Configurations::get<bool>("graphics.vsync"),
                Configurations::get<bool>("controls.gamepad-enabled"),
                Configurations::get<bool>("controls.gamepad-detection"),
                Configurations::get<bool>("controls.axis-as-dpad"),
                Configurations::get<float>("controls.axis-sensitivity"),
                8, // depth
                0, // stencil
                0, // antialiasing
                2, // GL major version
                0))), // GL minor version
        paused(false),
        pausing_enabled(true),
        magnification(Configurations::get<float>("debug.magnification")),
        style(xd::vec4(1.0f, 1.0f, 1.0f, 1.0f), Configurations::get<int>("font.size")),
        current_scripting_interface(nullptr),
        text_renderer(),
        editor_ticks(0),
        editor_size(1, 1) {
    // Set members
    clock = std::make_unique<Clock>(*this);
    camera = std::make_unique<Camera>(*this);
    shake_decorator = std::make_unique<Shake_Decorator>(*this);
    // Listen to config changes
    Configurations::add_observer("Game",
        [this](const std::string& key) {
            this->pimpl->on_config_change(key);
        }
    );
    // Default volumes
    set_global_music_volume(Configurations::get<float>("audio.music-volume"));
    set_global_sound_volume(Configurations::get<float>("audio.sound-volume"));
    // Setup fonts
    LOGGER_I << "Setting up fonts";
    style.outline(1, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f))
        .line_height(Configurations::get<float>("font.line-height"))
        .force_autohint(true);
    pimpl->debug_style.line_height(12.0f).force_autohint(true);
    auto font_file = Configurations::get<std::string>("font.default");
    auto bold_font_file = Configurations::get<std::string>("font.bold");
    auto italic_font_file = Configurations::get<std::string>("font.italic");
    if (!file_utilities::file_exists(font_file)) {
        throw std::runtime_error("Couldn't read font file " + font_file);
    }
    font = pimpl->asset_manager.load<xd::font>(font_file);
    if (!bold_font_file.empty()) {
        if (file_utilities::file_exists(bold_font_file)) {
            font->link_font("bold", pimpl->asset_manager.load<xd::font>(bold_font_file));
        } else {
            LOGGER_W << "Couldn't read bold font file " << bold_font_file;
        }
    }
    if (!italic_font_file.empty()) {
        if (file_utilities::file_exists(italic_font_file)) {
            font->link_font("italic", pimpl->asset_manager.load<xd::font>(italic_font_file));
        } else {
            LOGGER_W << "Couldn't read italic font file " << bold_font_file;
        }
    }

    if (editor_mode)
        return;

    window->set_gamma(Configurations::get<float>("graphics.gamma"));
    map = Map::load(*this, Configurations::get<std::string>("startup.map"));
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
    // Play background music
    auto bg_music = map->get_bg_music_filename();
    if (pimpl->audio && !bg_music.empty() && bg_music != "false") {
        load_music(map->get_bg_music_filename());
        music->set_looping(true);
        music->play();
    }
    // Track player by camera
    camera->set_object(player.get());
    // Bind game keys
    pimpl->process_keymap(*this, window.get());
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
    int logic_fps = Configurations::get<int>("debug.logic-fps");
    window->register_tick_handler(std::bind(&Game::frame_update, this), 1000 / logic_fps);
    // Setup shader, if any
    camera->set_shader(Configurations::get<std::string>("graphics.vertex-shader"),
        Configurations::get<std::string>("graphics.fragment-shader"));
}

Game::~Game() {
    Configurations::remove_observer("Game");
}

xd::audio* Game::get_audio() const {
    return pimpl->audio;
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
    if (pimpl->audio)
        pimpl->audio->update();

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
        else if (Configurations::get<bool>("game.pause-unfocused") && !window->focused()) {
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
    if (pimpl->next_map.empty())
        map->update();
    pimpl->process_config_changes(*this, window.get());
    // Switch map if needed
    if (!pimpl->next_map.empty())
        load_map(pimpl->next_map);
}

void Game::render() {
    // Window size changes are asynchronous in X11 so we keep polling the size
    camera->set_size(width(), height());
    camera->render();
    if (pimpl->editor_mode) return;
    // Draw FPS
    auto height = static_cast<float>(game_height());
    if (pimpl->show_fps) {
        text_renderer.render(*font, pimpl->debug_style,
            camera->get_geometry().projection().get(),5, 10,
            "FPS: " + std::to_string(fps()));
    }
    // Draw game time
    if (pimpl->show_time) {
        auto seconds = std::to_string(clock->seconds());
        text_renderer.render(*font, pimpl->debug_style,
            camera->get_geometry().projection().get(), 5, 20, seconds);
    }
    window->swap();
}

void Game::pause() {
    paused = true;
    pimpl->pause_start_time = window->ticks();
    pimpl->was_stopped = clock->stopped();
    clock->stop_time();
    if (music && Configurations::get<bool>("audio.mute-on-pause")) {
        pimpl->music_was_paused = music->paused();
        if (!pimpl->music_was_paused)
            music->pause();
    }
    camera->set_shader(Configurations::get<std::string>("graphics.pause-vertex-shader"),
        Configurations::get<std::string>("graphics.pause-fragment-shader"));
    if (pimpl->pause_scripting_interface) {
        auto pause_script = Configurations::get<std::string>("game.pause-script");
        run_script(file_utilities::read_file(pause_script));
    }
}

void Game::resume(const std::string& script) {
    paused = false;
    pimpl->total_paused_time += window->ticks() - pimpl->pause_start_time;
    if (!pimpl->was_stopped)
        clock->resume_time();
    if (music && !pimpl->music_was_paused && Configurations::get<bool>("audio.mute-on-pause"))
        music->play();
    camera->set_shader(Configurations::get<std::string>("graphics.vertex-shader"),
        Configurations::get<std::string>("graphics.fragment-shader"));
    if (!script.empty()) {
        run_script(script);
    }
}

void Game::exit() {
    pimpl->exit_requested = true;
}

void Game::set_size(int width, int height) {
    LOGGER_I << "Setting screen size to " << width << ", " << height;
    if (pimpl->editor_mode) {
        editor_size = xd::ivec2(width, height);
        pimpl->game_width = map->get_pixel_width() * magnification;
        pimpl->game_height = map->get_pixel_height() * magnification;
    } else {
        window->set_size(width, height);
    }
    camera->set_size(width, height);
}

xd::vec2 Game::get_monitor_size() const {
    return window ? window->get_size() : xd::vec2{0.0f, 0.0f};
}

std::vector<xd::vec2> Game::get_sizes() const {
    return window ? window->get_sizes() : std::vector<xd::vec2>{};
}

void Game::set_fullscreen(bool fullscreen) {
    if (!window || fullscreen == is_fullscreen()) return;
    LOGGER_I << "Changing window display to " << (fullscreen) ? "fullscreen" : "windowed";
    window->set_fullscreen(fullscreen);
    camera->set_size(width(), height());
}

float Game::game_width(bool magnified) const {
    return magnified ? pimpl->game_width / magnification : pimpl->game_width;
}

float Game::game_height(bool magnified) const {
    return magnified ? pimpl->game_height / magnification : pimpl->game_height;
}

void Game::set_magnification(float mag) {
    magnification = mag;
    camera->calculate_viewport(width(), height());
    camera->update_viewport();
}

std::vector<std::string> Game::triggered_keys() const {
    std::vector<std::string> results;
    auto keys = window->triggered_keys();
    for (auto key : keys) {
        if (key.type == xd::input_type::INPUT_GAMEPAD && key.device_id != get_gamepad_id()) continue;
        key.device_id = -1;
        results.push_back(pimpl->key_binder->get_key_name(key));
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

void Game::run_script(const std::string& script) {
    auto& si = paused ? pimpl->pause_scripting_interface : pimpl->scripting_interface;
    set_current_scripting_interface(si.get());
    si->run_script(script);
}

xd::lua::virtual_machine* Game::get_lua_vm() {
    return &pimpl->vm;
}

void Game::reset_scripting() {
    pimpl->reset_scripting = true;
}

void Game::load_music(const std::string& filename) {
    if (!pimpl->audio) return;
    if (music)
        music->stop();
    music = std::make_unique<xd::music>(*pimpl->audio, filename);
}

float Game::get_global_music_volume() const {
    if (!pimpl->audio) return 0.0f;
    return pimpl->audio->get_music_volume();
}

void Game::set_global_music_volume(float volume) const {
    if (!pimpl->audio) return;
    pimpl->audio->set_music_volume(volume);
}

float Game::get_global_sound_volume() const {
    if (!pimpl->audio) return 0.0f;
    return pimpl->audio->get_sound_volume();
}

void Game::set_global_sound_volume(float volume) const {
    if (!pimpl->audio) return;
    pimpl->audio->set_sound_volume(volume);
}

void Game::set_next_map(const std::string& filename, Direction dir) {
    pimpl->next_map = filename;
    pimpl->next_direction = dir == Direction::NONE ? player->get_direction() : dir;
    pimpl->next_position = std::nullopt;
}

void Game::set_next_map(const std::string& filename, float x, float y, Direction dir) {
    set_next_map(filename, dir);
    pimpl->next_position = xd::vec2{ x, y };
}

xd::asset_manager& Game::get_asset_manager() {
    return pimpl->asset_manager;
}

void Game::render_text(xd::font& font, xd::text_formatter& formatter,
        const xd::font_style& style, float x, float y, const std::string& text) {
    text_renderer.render_formatted(font, formatter, style, camera->get_geometry().projection().get(), x, y, text);
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

std::string Game::get_save_directory() const {
    return pimpl->get_save_directory();
}

void Game::save(std::string filename, Save_File& save_file) const {
    pimpl->cleanup_save_filename(filename);
    try {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) {
            throw std::runtime_error("Unable to open file for writing");
        }
        ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        ofs << save_file;
        LOGGER_I << "Saved file " << filename;
    } catch (const std::ios_base::failure & e) {
        LOGGER_E << "Error saving file " << filename << " - error code: " << e.code() << " - message: " << e.what();
    } catch (const config_exception& e) {
        LOGGER_E << "Error while saving config file - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error saving file " << filename << " - message: " << e.what();
    }
}

std::unique_ptr<Save_File> Game::load(std::string filename) {
    pimpl->cleanup_save_filename(filename);
    auto& state = pimpl->scripting_interface->lua_state();
    auto file = std::make_unique<Save_File>(state);
    try {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) {
            throw std::runtime_error("File doesn't exist or can't be opened");
        }
        ifs.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        ifs >> *file;
        LOGGER_I << "Loaded file " << filename;
    } catch (const std::ios_base::failure& e) {
        LOGGER_E << "Error loading file " << filename << " - error code: " << e.code() << " - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error loading file " << filename << " - message: " << e.what();
    }
    return file;
}

bool Game::save_config_file() const {
    try {
        file_utilities::save_config("config.ini");
        return true;
    } catch (const config_exception& e) {
        LOGGER_E << "Error while saving config file - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error saving config file - message: " << e.what();
    }
    return false;
}

bool Game::save_keymap_file() const {
    return pimpl->key_binder->save_keymap_file();
}

void Game::load_map(const std::string& filename) {
    map = Map::load(*this, filename);
    if (pimpl->editor_mode) return;
    // Add player to the map
    player->set_id(-1);
    player->set_triggered_object(nullptr);
    player->set_collision_object(nullptr);
    player->set_collision_area(nullptr);
    player->clear_linked_objects();
    auto start_pos = pimpl->next_position ? pimpl->next_position.value() : map->get_starting_position();
    auto bounding_box = player->get_bounding_box();
    start_pos.x -= bounding_box.x;
    start_pos.y -= bounding_box.y;
    player->set_position(start_pos);
    player->face(pimpl->next_direction);
    map->add_object(player);
    camera->set_object(player.get());
    // Play background music
    auto bg_music = map->get_bg_music_filename();
    auto playing_music = music ? music->get_filename() : "";
    if (bg_music == "false") {
        music.reset();
    } else if (!bg_music.empty() && bg_music != playing_music) {
        load_music(bg_music);
        music->set_looping(true);
        music->play();
    }
    // Run map startup scripts
    map->run_startup_scripts();
    pimpl->next_map = "";
}

void Game::new_map(xd::ivec2 map_size, xd::ivec2 tile_size) {
    map = std::make_unique<Map>(*this);
    map->resize(map_size, tile_size);
}

void Game::add_canvas(std::shared_ptr<Canvas> canvas) {
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
