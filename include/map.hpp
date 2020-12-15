#ifndef HPP_MAP
#define HPP_MAP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include "xd/system.hpp"
#include "xd/entity.hpp"
#include "xd/graphics/sprite_batch.hpp"
#include "vendor/rapidxml.hpp"
#include "xd/vendor/sol/forward.hpp"
#include "tmx_properties.hpp"
#include "direction.hpp"
#include "tileset.hpp"
#include "collision_record.hpp"
#include "collision_check_types.hpp"
#include "editable.hpp"
#include "layer_types.hpp"
#include "lua_object.hpp"

namespace xd {
    class music;
}
class Game;
class Map_Object;
class Canvas;
struct Object_Layer;
struct Tile_Layer;
struct Image_Layer;
struct Layer;
class Scripting_Interface;
class Map_Renderer;
class Map_Updater;

class Map : public xd::entity<Map>, public Editable, public Lua_Object {
public:
    friend class Map_Renderer;
    friend class Map_Updater;
    // Constructor and destructor
    explicit Map(Game& game);
    ~Map();
    // Get map name
    std::string get_name() const {
        if (properties.has_property("name"))
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
    // Check if object can move in given direction
    Collision_Record passable(const Map_Object& object, Direction direction,
        Collision_Check_Type check_type = Collision_Check_Type::BOTH,
        Collision_Record&& previous_record = Collision_Record{});
    Collision_Record passable(const Map_Object& object, Direction direction,
        xd::vec2 position, float speed,
        Collision_Check_Type check_type = Collision_Check_Type::BOTH,
        Collision_Record&& previous_record = Collision_Record{});
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
    // Get object by name
    Map_Object* get_object(std::string name) const;
    // Get object by ID
    Map_Object* get_object(int id) const;
    // Get all objects
    std::unordered_map<int, std::shared_ptr<Map_Object>>& get_objects() noexcept {
        return objects;
    }
    // Delete object
    void delete_object(const std::string& name);
    void delete_object(int id);
    void delete_object(Map_Object* object);
    // Get number of layers
    int layer_count() const;
    // Get layer by index (starting from 1, lua convention)
    Layer* get_layer(int id) const;
    // Get layer by name
    Layer* get_layer(std::string name) const;
    // Get image layer by index (starting from 1, lua convention)
    Image_Layer* get_image_layer(int id) const;
    // Get image layer by name
    Image_Layer* get_image_layer(const std::string& name) const;
    // Add a new layer
    void add_layer(Layer_Type type);
    // Delete layer with given name
    void delete_layer(std::string name);
    // Add a canvas to the map
    void add_canvas(std::shared_ptr<Canvas> canvas);
    // Erase canvases that match a predicate function
    template<typename Predicate>
    void erase_canvases(Predicate func) {
        canvases.erase(std::remove_if(
            std::begin(canvases), std::end(canvases), func), std::end(canvases));
    }
    // Return a sorted list of canvases
    const std::vector<std::weak_ptr<Canvas>>& get_canvases();
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
    Game& get_game() noexcept {
        return game;
    }
    void set_property(const std::string& name, const std::string& value) {
        properties[name] = value;
    }
    std::string get_property(const std::string& name) const {
        return properties[name];
    }
    int get_width() const noexcept {
        return width;
    }
    int get_height() const noexcept {
        return height;
    }
    int get_tile_width() const noexcept {
        return tile_width;
    }
    int get_tile_height() const noexcept {
        return tile_height;
    }
    int get_pixel_width() const noexcept {
        return width * tile_width;
    }
    int get_pixel_height() const noexcept {
        return height * tile_height;
    }
    std::string get_filename() const {
        return filename;
    }
    xd::vec2 get_starting_position() const noexcept {
        return starting_position;
    }
    const Tileset& get_tileset(int index) const {
        return tilesets.at(index);
    }
    bool get_objects_moved() const noexcept {
        return objects_moved;
    }
    std::string get_bg_music_filename() const {
        return background_music;
    }
    void set_bg_music_filename(const std::string& music_filename) {
        background_music = music_filename;
    }
    std::string get_startup_scripts() const {
        return get_property("scripts");
    }
    void set_startup_scripts(const std::string& scripts) {
        set_property("scripts", scripts);
    }
    void set_objects_moved(bool moved) noexcept {
        objects_moved = moved;
    }
    bool is_changed() noexcept {
        return needs_redraw;
    }
private:
    // Game instance
    Game& game;
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
    // Map properties
    Tmx_Properties properties;
    // Scripting interface for map scripts
    std::unique_ptr<Scripting_Interface> scripting_interface;
    // Hash table of object IDs to objects
    std::unordered_map<int, std::shared_ptr<Map_Object>> objects;
    // Hash table of object names to IDs
    std::unordered_multimap<std::string, int> object_name_to_id;
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
    // List of canvases to draw
    std::vector<std::weak_ptr<Canvas>> canvases;
    // Background music
    std::string background_music;
    // Startup script filenames
    std::vector<std::string> start_scripts;
    // Do we need to redraw static elements?
    bool needs_redraw;
    // Did any objects move?
    bool objects_moved;
    // Is canvas list already sorted?
    bool canvases_sorted;
    // Remove object from ID and name hash tables
    void erase_object_references(Map_Object* object);
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
