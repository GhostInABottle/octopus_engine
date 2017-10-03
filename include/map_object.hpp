#ifndef HPP_MAP_OBJECT
#define HPP_MAP_OBJECT

#include <string>
#include <xd/system.hpp>
#include <xd/entity.hpp>
#include <xd/graphics/types.hpp>
#include "rapidxml.hpp"
#include "common.hpp"
#include "direction.hpp"
#include "collision_record.hpp"
#include "collision_check_types.hpp"
#include "sprite_holder.hpp"
#include "sprite.hpp"
#include "editable.hpp"

class Game;
struct Object_Layer;

class Map_Object : public xd::entity<Map_Object>, public Sprite_Holder, public Editable {
public:
    enum Draw_Order { BELOW, NORMAL, ABOVE };
    struct Script {
        Script() : is_global(false) {}
        // Script source text
        std::string source;
        // Is script a local (map) or global script?
        bool is_global;
    };
    // Map object onstructor
    Map_Object(Game& game, std::string name = "",
        xd::asset_manager* manager = nullptr, std::string sprite_file = "",
        xd::vec2 pos = xd::vec2(), Direction dir = Direction::DOWN);
    // Move in a direction and return collision object
    Collision_Record move(Direction move_dir, float pixels,
        Collision_Check_Types check_type = Collision_Check_Types::BOTH,
        bool change_facing = true);
    // Getters and setters
    Object_Layer* get_layer() const {
        return layer;
    }
    void set_layer(Object_Layer* layer) {
        this->layer = layer;
    }
    int get_id() const {
        return id;
    }
    void set_id(int id) {
        this->id = id;
    }
    std::string get_name() const {
        return name;
    }
    void set_name(const std::string& name) {
        this->name = name;
    }
    std::string get_type() const {
        return type;
    }
    void set_type(const std::string& type) {
        this->type = type;
    }
    xd::vec2 get_position() const {
        return position;
    }
    void set_position(xd::vec2 position) {
        this->position = position;
    }
    float get_x() const {
        return position.x;
    }
    void set_x(float x) {
        position.x = x;
    }
    float get_y() const {
        return position.y;
    }
    void set_y(float y) {
        position.y = y;
    }
    xd::vec2 get_size() const {
        return size;
    }
    void set_size(xd::vec2 size) {
        this->size = size;
    }
    xd::vec4 get_color() const {
        return color;
    }
    void set_color(xd::vec4 color) {
        this->color = color;
    }
    unsigned int get_gid() const {
        return gid;
    }
    float get_opacity() const {
        return opacity;
    }
    void set_opacity(float opacity) {
        this->opacity = opacity;
    }
    bool is_visible() const {
        return visible;
    }
    void set_visible(bool visible) {
        this->visible = visible;
    }
    bool is_disabled() const {
        return disabled;
    }
    void set_disabled(bool disabled) {
        this->disabled = disabled;
        if (state == "WALK")
            update_state("FACE");
    }
    bool is_stopped() const {
        return stopped;
    }
    void set_stopped(bool stopped) {
        this->stopped = stopped;
    }
    bool is_frozen() const {
        return frozen;
    }
    void set_frozen(bool frozen) {
        this->frozen = frozen;
    }
    bool is_passthrough() const {
        return passthrough;
    }
    void set_passthrough(bool passthrough) {
        this->passthrough = passthrough;
    }
    Direction get_direction() const {
        return direction;
    }
    void set_direction(Direction dir) {
        face(dir);
    }
    std::string get_pose_name() const {
        return pose_name;
    }
    std::string get_state() const {
        return state;
    }
    std::string get_trigger_script_source() const {
        return trigger_script.source;
    }
    void set_trigger_script_source(const std::string& script);
    std::string get_exit_script_source() const {
        return exit_script.source;
    }
    void set_exit_script_source(const std::string& script);
    Map_Object* get_collision_area() const {
        return collision_area;
    }
    void set_collision_area(Map_Object* area) {
        collision_area = area;
    }
    Map_Object* get_triggered_object() const {
        return triggered_object;
    }
    void set_triggered_object(Map_Object* obj) {
        triggered_object = obj;
    }
    Draw_Order get_draw_order() const {
        return draw_order;
    }
    void set_draw_order(Draw_Order order) {
        draw_order = order;
    }
    Sprite* get_sprite() {
        return sprite.get();
    }
    const Sprite* get_sprite() const {
        return sprite.get();
    }
    // Set sprite using map's asset manager for NPCs and global one for player
    void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "");
    // Set sprite using specified asset manager
    void set_sprite(Game& game, xd::asset_manager& manager, const std::string& filename, const std::string& pose_name = "");
    float get_speed() const {
        return speed;
    }
    void set_speed(float speed);
    // Update object's state
    void update_state(const std::string& new_state) {
        if (!frozen) {
            state = new_state;
            update_pose();
        }
    }
    // Get bounding box
    xd::rect get_bounding_box() const {
        if (sprite)
            return get_sprite()->get_bounding_box();
        else
            return xd::rect(0, 0, size[0], size[1]);
    }
    // Get position with bounding box
    xd::vec2 get_real_position() const {
        auto box = get_bounding_box();
        return xd::vec2(position.x + box.x, position.y + box.y);
    }
    // Get sprite angle
    int get_angle() const;
    // Set sprite angle
    void set_angle(int angle);
    // Set pose
    void set_pose(const std::string& pose_name = "", const std::string& state = "",
            Direction direction = Direction::NONE) {
        if (!pose_name.empty())
            this->pose_name = pose_name;
        if (!state.empty())
            this->state = state;
        if (direction != Direction::NONE)
            this->direction = direction;
        Sprite_Holder::set_pose(this->pose_name, this->state, this->direction);
    }
    // Face another object
    void face(const Map_Object& other);
    // Face a certain spot
    void face(float x, float y);
    // Face a certain direction
    void face(Direction dir);
    // Run the object's activation script
    void run_trigger_script() {
        run_script(trigger_script);
    }
    // Run exit script (for areas)
    void run_exit_script() {
        run_script(exit_script);
    }
    // Serialize object to TMX data
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc);
    // Load the object from TMX data
    static std::unique_ptr<Map_Object> load(rapidxml::xml_node<>& node,
        Game& game, xd::asset_manager& manager);
private:
    // Game instance
    Game& game;
    // Associated map layer
    Object_Layer* layer;
    // Unique ID
    int id;
    // Name of the object
    std::string name;
    // Type of the object
    std::string type;
    // Object's position
    xd::vec2 position;
    // Object's size
    xd::vec2 size;
    // Object's tint color
    xd::vec4 color;
    // Optional reference to a tile
    unsigned int gid;
    // Object opacity
    float opacity;
    // Is object visible?
    bool visible;
    // Does the player ignore user input?
    bool disabled;
    // Stop moving at all?
    bool stopped;
    // Are movement state changes ignored?
    bool frozen;
    // Can object pass through obstaces?
    bool passthrough;
    // Object direction
    Direction direction;
    // Current pose name
    std::string pose_name;
    // Current pose state
    std::string state;
    // Script executed when object is triggered
    Script trigger_script;
    // Script executed when exiting an area
    Script exit_script;
    // Area the object is inside
    Map_Object* collision_area;
    // Last object activated by this one
    Map_Object* triggered_object;
    // Object properties
    Properties properties;
    // How object is drawn relative to other objects in layer
    Draw_Order draw_order;
    // Optional sprite representing the object
    Sprite::ptr sprite;
    // Object speed
    float speed;
    // Update current pose
    void update_pose() {
        Sprite_Holder::set_pose(pose_name, state, direction);
    }
    // Run a script
    void run_script(const Script& script);
};

#endif
