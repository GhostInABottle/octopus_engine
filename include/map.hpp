#ifndef HPP_MAP
#define HPP_MAP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <xd/config.hpp>
#include <xd/system.hpp>
#include <xd/entity.hpp>
#include <xd/graphics/sprite_batch.hpp>
#include <xd/asset_manager.hpp>
#include <xd/types.hpp>
#include "rapidxml.hpp"
#include "common.hpp"
#include "direction.hpp"
#include "tileset.hpp"
#include "collision_record.hpp"
#include "collision_check_types.hpp"
#include "editable.hpp"
#include "layer_types.hpp"

namespace xd {
    class music;
}
class Game;
class Map_Object;
class Canvas;
struct Object_Layer;
struct Tile_Layer;
struct Layer;
class Scripting_Interface;
class Map_Renderer;
class Map_Updater;

class Map : public xd::entity<Map>, public Editable {
public:
    friend class Map_Renderer;
    friend class Map_Updater;
    typedef std::shared_ptr<Map_Object> Object_Ptr;
    typedef std::unordered_map<int, Object_Ptr> Object_Map;
    // Constructor and destructor
    Map(Game& game);
    ~Map();
    // Get map name
    std::string get_name() const {
        if (properties.find("name") != properties.end())
            return properties.at("name");
        else
            return "unnamed map";
    }
    // Set name
    void set_name(const std::string& name) {
        properties["name"] = name;
    }
    // Run a script (limited to this map)
    void run_script(const std::string& script);
    // Run map's startup scripts
    void run_startup_scripts();
    // Check if object can move in given direction
    Collision_Record passable(const Map_Object& object, Direction direction,
        Collision_Check_Types check_type = Collision_Check_Types::BOTH);
    Collision_Record passable(const Map_Object& object, Direction direction,
        xd::vec2 position, float speed,
        Collision_Check_Types check_type = Collision_Check_Types::BOTH);
    // Check if a particular tile is passable
    bool tile_passable(int x, int y);
    // Get number of objects
    int object_count();
    // Add a new object to object layer at index (or center object layer if -1)
    Map_Object* add_object(Object_Ptr object,
        int layer_index = -1, Object_Layer* layer = nullptr);
    Map_Object* add_object(Map_Object* object,
        int layer_index = -1, Object_Layer* layer = nullptr);
	// Create and add an object
	Map_Object* add_new_object(std::string name = "", std::string sprite_file = "",
		xd::vec2 pos = xd::vec2(), Direction dir = Direction::DOWN);
    // Get object by name
    Map_Object* get_object(const std::string& name);
	// Get object by ID
	Map_Object* get_object(int id);
    // Get all objects
    Object_Map& get_objects() {
        return objects;
    }
    // Delete object
    void delete_object(const std::string& name);
	void delete_object(int id);
    void delete_object(Map_Object* object);
    // Get number of layers
    int layer_count();
    // Get layer by index (starting from 1, lua convention)
    Layer* get_layer(int id);
    // Get layer by name
    Layer* get_layer(const std::string& name);
	// Add a new layer
	void add_layer(Layer_Types type);
	// Delete layer with given name
    void delete_layer(const std::string& name);
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
    std::string get_filename() const {
        return filename;
    }
    xd::asset_manager& get_asset_manager() {
        return asset_manager;
    }
    const Tileset& get_tileset(int index) const {
        return tilesets[index];
    }
    std::vector<std::shared_ptr<Canvas>>& get_canvases() {
        return canvases;
    }
    bool get_objects_moved() const {
        return objects_moved;
    }
    std::string get_bg_music_filename() const {
        return background_music;
    }
    void set_bg_music_filename(const std::string& filename) {
        background_music = filename;
    }
    std::string get_startup_scripts() const {
        if (properties.find("scripts") != properties.end()) {
            return properties.at("scripts");
        } else {
            return "";
        }
    }
    void set_startup_scripts(const std::string& scripts) {
        properties["scripts"] = scripts;
    }
    void set_objects_moved(bool moved) {
        objects_moved = moved;
    }
    bool is_changed() {
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
    // Map properties
    Properties properties;
    // Scripting interface for map scripts
    std::unique_ptr<Scripting_Interface> scripting_interface;
    // Texture asset manager
    xd::asset_manager asset_manager;
    // Hash table of object IDs to objects
    Object_Map objects;
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
    std::vector<std::shared_ptr<Canvas>> canvases;
    // Background music
    std::string background_music;
    // Startup script files
    std::vector<std::string> start_scripts;
    // Do we need to redraw static elements?
    bool needs_redraw;
    // Did any objects move?
    bool objects_moved;
	// Remove object from ID and name hash tables
	void erase_object_references(Map_Object* object);
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
