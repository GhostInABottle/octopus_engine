#ifndef HPP_GAME
#define HPP_GAME

#include <vector>
#include <memory>
#include <string>
#include <xd/system.hpp>
#include <xd/graphics/types.hpp>
#include <xd/graphics/simple_text_renderer.hpp>
#include <xd/graphics/font.hpp>
#include <xd/audio.hpp>
#include "direction.hpp"

class Camera;
class Map;
class Map_Object;
class Scripting_Interface;
class NPC;
class Clock;
namespace xd {
    class shader_program;
    class asset_manager;
}

class Game : public xd::window {
public:
    Game();
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
    // Run a script
    void run_script(const std::string& script);
    // Set or get the current scripting interface
    void set_current_scripting_interface(Scripting_Interface* si) {
        current_scripting_interface = si;
    }
    Scripting_Interface* get_current_scripting_interface() {
        return current_scripting_interface;
    }
    // Play some music
    xd::music::ptr load_music(const std::string& filename);
    // Play some sound effect
    xd::sound::ptr load_sound(const std::string& filename);
    // Get the music currently playing
    xd::music::ptr playing_music() { return music; }
    // Modelview projection matrix
    xd::mat4 get_mvp() const { return geometry.mvp(); }
    // Load map file with given name and set as current map
    void set_next_map(const std::string& filename, float x, float y, Direction dir);
    // Get the map
    Map* get_map() { return map.get(); }
    // Get the camera
    Camera* get_camera() { return camera.get(); }
    // Get the player
    Map_Object* get_player() { return player.get(); }
    // Get global asset manager
    xd::asset_manager& get_asset_manager();
    // Get text renderer
    xd::simple_text_renderer& get_text_renderer() { return text_renderer; }
    // Get font
    xd::font* get_font() { return font.get(); }
    // Get all NPCs
    std::vector<std::unique_ptr<NPC>>& get_npcs() { return npcs; }
    // Get NPC with given name
    NPC* get_npc(const std::string& name);
    // Get clock
    Clock* get_clock() { return clock.get(); }
    // Time elapsed since game started (in ms)
    int ticks() const;
    // Apply a certain shader
    void set_shader(const std::string& vertex, const std::string& fragment);
    // Process key-mapping string
    void process_keymap();
    // Game width
    static int game_width;
    // Game height
    static int game_height;
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
    std::unique_ptr<Clock> clock;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Map> map;
    Scripting_Interface* current_scripting_interface;
    xd::transform_geometry geometry;
    std::shared_ptr<Map_Object> player;
    xd::music::ptr music;
    xd::font::ptr font;
    xd::simple_text_renderer text_renderer;
    std::vector<std::unique_ptr<NPC>> npcs;
    void load_map(const std::string& filename);
};

#endif
