#ifndef HPP_MAP
#define HPP_MAP

#include "../direction.hpp"
#include "../interfaces/editable.hpp"
#include "../scripting/lua_object.hpp"
#include "../vendor/rapidxml.hpp"
#include "../xd/asset_manager.hpp"
#include "../xd/entity.hpp"
#include "../xd/vendor/sol/forward.hpp"
#include "collision_check_options.hpp"
#include "collision_record.hpp"
#include "layers/layer_types.hpp"
#include "tileset.hpp"
#include "tmx_properties.hpp"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace xd {
    class sound;
}
class Game;
class Map_Object;
class Base_Canvas;
class Object_Layer;
class Tile_Layer;
class Image_Layer;
class Layer;
class Scripting_Interface;
class Map_Renderer;
class Map_Updater;

class Map : public xd::entity<Map>, public Editable, public Tmx_Object {
public:
    friend class Map_Renderer;
    friend class Map_Updater;
    struct Canvas_Ref {
        int id;
        std::weak_ptr<Base_Canvas> ptr;
        Canvas_Ref(int id, std::weak_ptr<Base_Canvas> ptr) : id(id), ptr(ptr) {}
    };
    // Constructor and destructor
    explicit Map(Game& game);
    ~Map();
    // Get map name
    std::string get_name() const {
        if (properties.contains("name"))
            return properties["name"];
        else
            return "unnamed map";
    }
    // Set name
    void set_name(const std::string& name) {
        properties["name"] = name;
    }
    // Run a script (limited to this map)
    void run_script(const std::string& script) {
        run_script_impl(script, false);
    }
    // Load and run a script file (limited to this map)
    void run_script_file(const std::string& filename) {
        run_script_impl(filename, true);
    }
    // Run a Lua function
    void run_function(const sol::protected_function& function);
    // Run map's startup scripts
    void run_startup_scripts();
    // Is the Lua scheduler paused?
    bool is_script_scheduler_paused() const;
    // Pause or resume the Lua scheduler
    void set_script_scheduler_paused(bool paused);
    // Check if object can move in given direction
    Collision_Record passable(Collision_Check_Options options) const;
    // Check if a particular tile is passable
    bool tile_passable(int x, int y) const noexcept;
    // Get number of objects
    int object_count() const noexcept;
    // Add an object to object layer (or center object layer if no layer)
    Map_Object* add_object(const std::shared_ptr<Map_Object>& object, Object_Layer* layer = nullptr);
    // Create and add an object
    Map_Object* add_new_object(std::optional<std::string> name = std::nullopt,
        std::optional<std::string> sprite_file = std::nullopt,
        std::optional<xd::vec2> pos = std::nullopt,
        std::optional<Direction> dir = std::nullopt,
        std::optional<Object_Layer*> layer = std::nullopt);
    // Change an object's layer
    void move_object_to_layer(Map_Object* object, Object_Layer* layer);
    // Get object by name
    Map_Object* get_object(std::string name) const;
    // Get object by ID
    Map_Object* get_object(int id) const;
    // Get all objects
    std::unordered_map<int, std::shared_ptr<Map_Object>>& get_objects() {
        return objects;
    }
    // Delete object
    void delete_object(const std::string& name);
    void delete_object(int id);
    void delete_object(Map_Object* object);
    // Get number of layers
    int layer_count() const;
    // Get layer by index (starting from 1, lua convention)
    Layer* get_layer_by_index(int index) const;
    // Get layer by unique TMX ID
    Layer* get_layer_by_id(int id) const;
    // Get layer by name
    Layer* get_layer_by_name(std::string name) const;
    // Get image layer by index (starting from 1, lua convention)
    Image_Layer* get_image_layer_by_index(int index) const;
    // Get image layer by unique TMX ID
    Image_Layer* get_image_layer_by_id(int id) const;
    // Get image layer by name
    Image_Layer* get_image_layer_by_name(const std::string& name) const;
    // Get object layer by index (starting from 1, lua convention)
    Object_Layer* get_object_layer_by_index(int index) const;
    // Get object layer by unique TMX ID
    Object_Layer* get_object_layer_by_id(int id) const;
    // Get object layer by name
    Object_Layer* get_object_layer_by_name(const std::string& name) const;
    // Add a new layer
    void add_layer(Layer_Type type);
    // Add an existing layer
    void add_layer(std::shared_ptr<Layer> layer);
    // Delete layer with given name
    void delete_layer(std::string name);
    // Add a canvas to the map
    void add_canvas(std::shared_ptr<Base_Canvas> canvas);
    // Erase canvases that match a predicate function
    void erase_canvases();
    // Return a sorted list of canvases
    const std::vector<Canvas_Ref>& get_canvases();
    // Get a canvas by its ID
    Base_Canvas* get_canvas(int id) const;
    // Resize map and layers
    void resize(xd::ivec2 map_size, xd::ivec2 tile_size);
    // Save map to specified file name
    void save(std::string filename);
    // Save map to XML document
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc);
    // Load map from a TMX file
    static std::unique_ptr<Map> load(Game& game, const std::string& filename);
    // Load map from a TMX map node
    static std::unique_ptr<Map> load(Game& game, rapidxml::xml_node<>& node);
    // Getters and setters
    Game& get_game() {
        return game;
    }
    Lua_Object& get_lua_data() {
        return lua_data;
    }
    int get_width() const {
        return width;
    }
    int get_height() const {
        return height;
    }
    int get_tile_width() const {
        return tile_width;
    }
    int get_tile_height() const {
        return tile_height;
    }
    int get_pixel_width() const {
        return width * tile_width;
    }
    int get_pixel_height() const {
        return height * tile_height;
    }
    std::string get_filename() const {
        return filename;
    }
    xd::vec2 get_starting_position() const {
        return starting_position;
    }
    void set_starting_position(xd::vec2 new_pos) {
        starting_position = new_pos;
    }
    xd::asset_manager& get_asset_manager() {
        return asset_manager;
    }
    const Tileset& get_tileset(int index) const {
        return tilesets.at(index);
    }
    bool get_draw_outlines() const {
        return draw_object_outlines;
    }
    void set_draw_outlines(bool draw) {
        draw_object_outlines = draw;
    }
    bool get_objects_moved() const {
        return objects_moved;
    }
    std::string get_bg_music_filename() const {
        return background_music_filename;
    }
    void set_bg_music_filename(const std::string& music_filename) {
        background_music_filename = music_filename;
    }
    float get_bg_music_volume() const {
        return background_music_volume;
    }
    void set_bg_music_volume(float music_volume) {
        background_music_volume = music_volume;
    }
    std::string get_bg_ambient_filename() const {
        return background_ambient_filename;
    }
    void set_bg_ambient_filename(const std::string& ambient_filename) {
        background_ambient_filename = ambient_filename;
    }
    float get_bg_ambient_volume() const {
        return background_ambient_volume;
    }
    void set_bg_ambient_volume(float ambinet_volume) {
        background_ambient_volume = ambinet_volume;
    }
    std::string get_startup_scripts() const {
        return get_property("scripts");
    }
    void set_startup_scripts(const std::string& scripts) {
        set_property("scripts", scripts);
    }
    void set_objects_moved(bool moved) {
        objects_moved = moved;
    }
    bool is_changed() const {
        return needs_redraw;
    }
    void sort_canvases() {
        canvases_sorted = false;
    }
    typedef std::unordered_map<std::string, std::vector<std::shared_ptr<xd::sound>>> Audio_Cache;
    Audio_Cache& get_audio_cache() {
        return audio_cache;
    }
    int next_typewriter_slot() {
        return ++last_typewriter_slot;
    }
private:
    // Game instance
    Game& game;
    // Allows defining extra Lua properties on the object
    Lua_Object lua_data;
    // Width in tiles
    int width;
    // Height in tiles
    int height;
    // Width of each tile in pixels
    int tile_width;
    // Height of each tile in pixels
    int tile_height;
    // Map file name
    std::string filename;
    // Counter to set IDs of new objects
    int next_object_id;
    // Starting position for player
    xd::vec2 starting_position;
    // Scripting interface for map scripts
    std::unique_ptr<Scripting_Interface> scripting_interface;
    // Hash table of object IDs to objects
    std::unordered_map<int, std::shared_ptr<Map_Object>> objects;
    // Hash table of object names to IDs
    std::unordered_map<std::string, int> object_name_to_id;
    // Asset cache for textures and sprites
    xd::asset_manager asset_manager;
    // List of map tilesets
    std::vector<Tileset> tilesets;
    // Obstruction data tileset
    Tileset* collision_tileset;
    // Object layer
    std::vector<Object_Layer*> object_layers;
    // Obstruction data layer
    Tile_Layer* collision_layer;
    // List of map layers
    std::vector<std::shared_ptr<Layer>> layers;
    // Lookup table of layer IDs to layer pointers
    std::unordered_map<int, Layer*> layers_by_id;
    // List of canvases to draw
    std::vector<Canvas_Ref> canvases;
    // Lookup table of canvas IDs to canvases
    std::unordered_map<int, std::weak_ptr<Base_Canvas>> canvases_by_id;
    // Background music
    std::string background_music_filename;
    float background_music_volume;
    // Background ambient
    std::string background_ambient_filename;
    float background_ambient_volume;
    // Startup script filenames
    std::vector<std::string> start_scripts;
    // Should object outlines be drawn?
    bool draw_object_outlines;
    // Do we need to redraw static elements?
    bool needs_redraw;
    // Did any objects move?
    bool objects_moved;
    // Is canvas list already sorted?
    bool canvases_sorted;
    // Cached sounds/audio for the map
    Audio_Cache audio_cache;
    // Used to uniquely identify typewriter text effects
    int last_typewriter_slot;
    // Remove object from ID and name hash tables
    void erase_object_references(const Map_Object* object);
    void run_script_impl(const std::string& script_or_filename, bool is_filename);
};

class Map_Renderer : public xd::render_component<Map> {
public:
    virtual void render(Map& map);
};

class Map_Updater : public xd::logic_component<Map> {
public:
    virtual void update(Map& map);
};


#endif
