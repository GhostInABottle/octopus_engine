#ifndef HPP_GAME
#define HPP_GAME

#include <vector>
#include <memory>
#include <string>
#include "xd/system.hpp"
#include "xd/graphics/types.hpp"
#include "xd/graphics/simple_text_renderer.hpp"
#include "xd/graphics/font.hpp"
#include "direction.hpp"

class Camera;
class Map;
class Map_Object;
class Scripting_Interface;
class Canvas;
class Clock;
class Save_File;
class Shake_Decorator;
namespace xd {
    class asset_manager;
    class music;
    namespace lua {
        class virtual_machine;
    }
}

class Game {
public:
    explicit Game(bool editor_mode = false);
    ~Game();
    // Main game loop
    void run();
    // Logic update
    void frame_update();
    // Render the scene
    void render();
    // Pause game
    void pause();
    // Resume game
    void resume();
    // Exit game
    void exit();
    // Screen dimensions
    int width() const {
        return window ? window->framebuffer_width() : editor_size.x;
    }
    int height() const {
        return window ? window->framebuffer_height() : editor_size.y;
    }
    // Manually set window size (for editor)
    void set_size(int width, int height);
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
    bool pressed(const std::string& key) const;
    // Check if main axis is pressed in a direction
    bool pressed(Direction direction) const;
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
    // Run a script
    void run_script(const std::string& script);
    // Set or get the current scripting interface
    void set_current_scripting_interface(Scripting_Interface* si) {
        current_scripting_interface = si;
    }
    Scripting_Interface* get_current_scripting_interface() {
        return current_scripting_interface;
    }
    // Get the shared Lua virtual machine
    xd::lua::virtual_machine* get_lua_vm();
    // Play some music
    std::shared_ptr<xd::music> load_music(const std::string& filename);
    // Get the music currently playing
    std::shared_ptr<xd::music> playing_music() { return music; }
    // Load map file and set as current map at the end of the frame
    void set_next_map(const std::string& filename, float x, float y, Direction dir);
    // Load the map right away
    void load_map(const std::string& filename);
    // Get the map
    Map* get_map() { return map.get(); }
    // Create a new map
    void new_map(xd::ivec2 map_size, xd::ivec2 tile_size);
    // Add a canvas to current map
    void add_canvas(std::shared_ptr<Canvas> canvas);
    // Get the camera
    Camera* get_camera() { return camera.get(); }
    // Get the player
    Map_Object* get_player() { return player.get(); }
    // Get global asset manager
    xd::asset_manager& get_asset_manager();
    // Get text renderer
    xd::simple_text_renderer& get_text_renderer() { return text_renderer; }
    // Get font
    std::shared_ptr<xd::font> get_font() { return font; }
    // Get style
    const xd::font_style& get_font_style() { return style; }
    // Get decorator for shaking text
    Shake_Decorator* get_shake_decorator() { return shake_decorator.get(); }
    // Get clock
    Clock* get_clock() { return clock.get(); }
    // Is time stopped
    bool stopped() const;
    // Total game time in seconds
    int seconds() const;
    // Time elapsed since game started (in ms)
    int ticks() const;
    // Manually set ticks
    void set_ticks(int ticks) {
        editor_ticks = ticks;
    }
    // Save game
    void save(const std::string& filename, Save_File& save_file) const;
    // Load game
    std::unique_ptr<Save_File> load(const std::string& filename);
    // Get current gamepad ID
    int get_gamepad_id() const;
    // Process key-mapping string
    void process_keymap();
private:
    std::unique_ptr<xd::window> window;
    float magnification;
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
    std::unique_ptr<Clock> clock;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Map> map;
    Scripting_Interface* current_scripting_interface;
    std::shared_ptr<Map_Object> player;
    std::shared_ptr<xd::music> music;
    std::shared_ptr<xd::font> font;
    xd::font_style style;
    xd::simple_text_renderer text_renderer;
    int editor_ticks;
    xd::ivec2 editor_size;
    std::unique_ptr<Shake_Decorator> shake_decorator;
};

#endif
