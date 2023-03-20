#ifndef HPP_GAME
#define HPP_GAME

#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "xd/graphics/framebuffer.hpp"
#include "xd/graphics/font.hpp"
#include "xd/graphics/simple_text_renderer.hpp"
#include "xd/graphics/types.hpp"
#include "xd/system.hpp"
#include "xd/audio/channel_group_type.hpp"
#include "xd/vendor/sol/forward.hpp"
#include "direction.hpp"

class Camera;
class Map;
class Map_Object;
class Scripting_Interface;
class Base_Canvas;
class Clock;
class Audio_Player;
class Typewriter_Decorator;
class Key_Binder;

namespace xd {
    class audio;
    class asset_manager;
    namespace lua {
        class virtual_machine;
    }
}

class Game {
public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    explicit Game(const std::vector<std::string>& args, std::shared_ptr<xd::audio> audio, bool editor_mode = false);
    ~Game();

    // Get the active channel group for sound effects
    channel_group_type get_sound_group_type() const;
    // Main game loop
    void run();
    // Logic update
    void frame_update();
    // Render the scene
    void render();
    // Is the game running in editor mode?
    bool in_editor_mode() const;
    // Is the game currently paused?
    bool is_paused() const { return paused; }
    // Check if pausing is possible
    bool is_pausing_enabled() const { return pausing_enabled; }
    // Set whether the game will pause on button press or unfocus
    void set_pausing_enabled(bool value) { pausing_enabled = value; }
    // Pause game
    void pause();
    // Resume game
    void resume(const std::string& script = "");
    // Exit game
    void exit();
    // Window dimensions in screen coordinates
    int window_width() const {
        return window ? window->width() : editor_size.x;
    }
    int window_height() const {
        return window ? window->height() : editor_size.y;
    }
    // Framebuffer dimensions in pixels
    int framebuffer_width() const {
        return window ? window->framebuffer_width() : editor_size.x;
    }
    int framebuffer_height() const {
        return window ? window->framebuffer_height() : editor_size.y;
    }
    // Was the executable build in debug mode?
    bool is_debug() const {
#ifdef NDEBUG
        return false;
#else
        return true;
#endif
    }
    // Manually set window resolution
    void set_size(int width, int height);
    // Returns the current resolution
    xd::vec2 get_monitor_size() const;
    // Returns list of supported resolutions
    std::vector<xd::vec2> get_sizes() const;
    // Is game in fullscreen mode?
    bool is_fullscreen() const {
        return window ? window->is_fullscreen() : false;
    }
    // Toggle fullscreen mode
    void set_fullscreen(bool fullscreen);
    // Game dimensions
    float game_width(bool magnified = true) const;
    float game_height(bool magnified = true) const;
    // Get screen magnification
    float get_magnification() const {
        return magnification;
    }
    // Set screen magnification
    void set_magnification(float mag);
    // Frames per seconds
    int fps() const {
        return window->fps();
    }
    // Number of frames since the beginning
    int frame_count() const {
        return window->frame_count();
    }
    // Is key currently pressed
    bool pressed(const xd::key& key) const {
        return window->pressed(key, get_gamepad_id());
    }
    bool pressed(const std::string& key) const {
        return window->pressed(key, get_gamepad_id());
    }
    // Was any key triggered since last update?
    bool triggered() const {
        return window->triggered();
    }
    // Was key triggered since last update?
    bool triggered(const xd::key& key) const {
        return window->triggered(key, get_gamepad_id());
    }
    bool triggered(const std::string& key) const {
        return window->triggered(key, get_gamepad_id());
    }
    // Was key triggered? (Un-trigger it if it was)
    bool triggered_once(const xd::key& key) {
        return window->triggered_once(key, get_gamepad_id());
    }
    bool triggered_once(const std::string& key) {
        return window->triggered_once(key, get_gamepad_id());
    }
    // Get physical names of triggered keys
    std::vector<std::string> triggered_keys() const;
    // Bind physical key to virtual key name
    void bind_key(const std::string& physical_name, const std::string& virtual_name);
    void bind_key(const xd::key& physical_key, const std::string& virtual_key) {
        window->bind_key(physical_key, virtual_key);
    }
    // Unbind physical key
    void unbind_physical_key(const xd::key& physical_key) {
        window->unbind_key(physical_key);
    }
    void unbind_physical_key(const std::string& physical_key);
    // Unbind virtual key
    void unbind_virtual_key(const std::string& virtual_key);
    // Get bound physical key names for a virtual key
    std::vector<std::string> get_bound_keys(const std::string& virtual_name) const;
    // Start recording character/text input
    void begin_character_input() const { window->begin_character_input(); }
    // Stop recording character input and reset the buffer
    std::string end_character_input() { return window->end_character_input(); }
    // Get the currently stored buffer of characters
    std::string character_input() const { return window->character_input(); }
    // Get printable name for a key
    std::string get_key_name(const std::string& physical_key) const;
    xd::input_type get_last_input_type() const { return window->last_input_type(); }
    // Run a script
    void run_script(const std::string& script);
    // Run a script file
    void run_script_file(const std::string& filename);
    // Run a Lua function
    void run_function(const sol::protected_function& function);
    // Set or get the current scripting interface
    void set_current_scripting_interface(Scripting_Interface* si) {
        current_scripting_interface = si;
    }
    Scripting_Interface* get_current_scripting_interface() {
        return current_scripting_interface;
    }
    // Get the shared Lua virtual machine
    xd::lua::virtual_machine* get_lua_vm();
    // Reset the scripting interface and run startup scripts again
    void reset_scripting();
    // Is the Lua scheduler paused?
    bool is_script_scheduler_paused() const;
    // Pause or resume the Lua scheduler
    void set_script_scheduler_paused(bool paused);
    // Get the cached audio subsystem
    Audio_Player& get_audio_player();
    // Load map file and set as current map at the end of the frame
    void set_next_map(const std::string& filename,
        Direction dir = Direction::NONE,
        std::optional<xd::vec2> pos = std::nullopt,
        std::optional<std::string> music = std::nullopt);
    // Load the map specified by set_next_map
    void load_next_map();
    // Get the map
    Map* get_map() { return map.get(); }
    // Create a new map
    void new_map(xd::ivec2 map_size, xd::ivec2 tile_size);
    // Add a canvas to current map
    void add_canvas(std::shared_ptr<Base_Canvas> canvas);
    // Get the camera
    Camera* get_camera() { return camera.get(); }
    // Get the player
    Map_Object* get_player() { return player.get(); }
    // Get global asset manager
    xd::asset_manager& get_asset_manager();
    // Get the framebuffer object
    xd::framebuffer& get_framebuffer() const { return *framebuffer; }
    // Create a font
    std::shared_ptr<xd::font> create_font(const std::string& filename);
    // Render some text
    void render_text(xd::font& font, const xd::font_style& style, float x, float y, const std::string& text);
    // Get font
    std::shared_ptr<xd::font> get_font() { return font; }
    // Get style
    const xd::font_style& get_font_style() { return style; }
    // Get width of text, in pixels
    float text_width(const std::string& text, xd::font* font = nullptr, const xd::font_style* style = nullptr);
    // Get clock
    Clock* get_clock() { return clock.get(); }
    // Is time stopped
    bool stopped() const;
    // Total game time in seconds
    int seconds() const;
    // Time elapsed since game started (in ms) not including pauses
    int ticks() const;
    // Manually set ticks
    void set_ticks(int ticks) {
        editor_ticks = ticks;
    }
    // Total time elapsed since game started (in ms)
    int window_ticks() { return window ? window->ticks() : editor_ticks; }
    // Get configured directory for Lua scripts
    std::string get_scripts_directory() const;
    // Check if gamepad config is enabled and at least one gamepad exists
    bool gamepad_enabled() const;
    // Get current gamepad ID
    int get_gamepad_id() const;
    // Get connected gamepad IDs and their display names
    std::unordered_map<int, std::string> gamepad_names() const {
        return window->joystick_names();
    }
    // Get current gamepad's name
    std::optional<std::string> get_gamepad_name() const;
    // Get string to be added to map object scripts
    std::string get_object_script_preamble() const;
    // Get arguments used to launch the game
    const std::vector<std::string>& get_command_line_args() const {
        return command_line_args;
    }
    Typewriter_Decorator& get_typewriter_decorator();
    Key_Binder& get_key_binder();
private:
    std::vector<std::string> command_line_args;
    // Window needs to be constructed before pimpl
    std::unique_ptr<xd::window> window;
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
    bool paused;
    bool pausing_enabled;
    float magnification;
    std::unique_ptr<Clock> clock;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Map> map;
    std::unique_ptr<xd::framebuffer> framebuffer;
    Scripting_Interface* current_scripting_interface;
    std::shared_ptr<Map_Object> player;
    std::shared_ptr<xd::font> font;
    xd::font_style style;
    int editor_ticks;
    xd::ivec2 editor_size;
};

#endif
