#ifndef HPP_MAP_OBJECT
#define HPP_MAP_OBJECT

#include <string>
#include <memory>
#include <optional>
#include <algorithm>
#include <vector>
#include "xd/system.hpp"
#include "xd/entity.hpp"
#include "xd/graphics/types.hpp"
#include "vendor/rapidxml.hpp"
#include "tmx_properties.hpp"
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
    enum class Draw_Order { BELOW, NORMAL, ABOVE };
    enum class Script_Context { MAP, GLOBAL };
    enum class Passthrough_Type { INITIATOR = 1, RECEIVER, BOTH };
    // Map object onstructor
    Map_Object(Game& game, const std::string& name = "", std::string sprite_file = "",
        xd::vec2 pos = xd::vec2(), Direction dir = Direction::DOWN);
    // Move in a direction and return collision object
    Collision_Record move(Direction move_dir, float pixels,
        Collision_Check_Types check_type = Collision_Check_Types::BOTH,
        bool change_facing = true);
    // Getters and setters
    Object_Layer* get_layer() const {
        return layer;
    }
    void set_layer(Object_Layer* new_layer) {
        layer = new_layer;
    }
    int get_id() const {
        return id;
    }
    void set_id(int new_id) {
        id = new_id;
    }
    std::string get_name() const {
        return name;
    }
    void set_name(const std::string& new_name);
    void set_property(const std::string& name, const std::string& value) {
        properties[name] = value;
    }
    std::string get_property(const std::string& name) const {
        return properties[name];
    }
    std::string get_type() const {
        return type;
    }
    void set_type(const std::string& new_type) {
        type = new_type;
    }
    xd::vec2 get_position() const {
        return position;
    }
    void set_position(xd::vec2 new_position) {
        position = new_position;
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
    xd::vec2 get_text_position() const {
        return get_position() + xd::vec2(16, -6);
    }
    xd::vec2 get_size() const {
        return size;
    }
    void set_size(xd::vec2 new_size) {
        size = new_size;
    }
    xd::vec4 get_color() const {
        return color;
    }
    void set_color(xd::vec4 new_color) {
        color = new_color;
    }
    xd::vec2 get_magnification() const {
        return magnification;
    }
    void set_magnification(xd::vec2 new_mag) {
        magnification = new_mag;
    }
    xd::vec2 get_sprite_magnification() const;
    unsigned int get_gid() const {
        return gid;
    }
    float get_opacity() const {
        return opacity;
    }
    void set_opacity(float new_opacity) {
        opacity = new_opacity;
    }
    bool is_visible() const {
        return visible;
    }
    void set_visible(bool new_visible) {
        visible = new_visible;
    }
    bool is_disabled() const {
        return disabled;
    }
    void set_disabled(bool new_disabled) {
        disabled = new_disabled;
        if (state == walk_state)
            update_state(face_state);
    }
    bool is_stopped() const {
        return stopped;
    }
    void set_stopped(bool new_stopped) {
        stopped = new_stopped;
    }
    bool is_frozen() const {
        return frozen;
    }
    void set_frozen(bool new_frozen) {
        frozen = new_frozen;
    }
    bool is_passthrough() const {
        return passthrough;
    }
    void set_passthrough(bool new_passthrough) {
        passthrough = new_passthrough;
    }
    Passthrough_Type get_passthrough_type() const {
        return passthrough_type;
    }
    void set_passthrough_type(Passthrough_Type type) {
        passthrough_type = type;
    }
    bool initiates_passthrough() const {
        return passthrough
            && (static_cast<int>(passthrough_type) & static_cast<int>(Passthrough_Type::INITIATOR)) != 0;
    }
    bool receives_passthrough() const {
        return passthrough
            && (static_cast<int>(passthrough_type) & static_cast<int>(Passthrough_Type::RECEIVER)) != 0;
    }
    bool overrides_tile_collision() const {
        return override_tile_collision;
    }
    void set_override_tile_collision(bool new_override) {
        override_tile_collision = new_override;
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
    std::string get_face_state() const {
        return face_state;
    }
    void set_face_state(const std::string& name) {
        face_state = name;
    }
    std::string get_walk_state() const {
        return walk_state;
    }
    void set_walk_state(const std::string& name) {
        walk_state = name;
    }
    Script_Context get_script_context() const {
        return script_context;
    }
    void set_script_context(Script_Context context) {
        script_context = context;
    }
    bool has_trigger_script() const {
        return !trigger_script.empty();
    }
    std::string get_trigger_script() const {
        return trigger_script;
    }
    void set_trigger_script(const std::string& script);
    bool has_touch_script() const {
        return !touch_script.empty();
    }
    std::string get_touch_script() const {
        return touch_script;
    }
    void set_touch_script(const std::string& script);
    bool has_leave_script() const {
        return !leave_script.empty();
    }
    std::string get_leave_script() const {
        return leave_script;
    }
    void set_leave_script(const std::string& script);
    bool has_any_script() const {
        return !trigger_script.empty() || !touch_script.empty() || !leave_script.empty();
    }
    bool is_player_facing() const {
        return player_facing;
    }
    void set_player_facing(bool facing) {
        player_facing = facing;
    }
    Map_Object* get_collision_object() const {
        return collision_object;
    }
    void set_collision_object(Map_Object* object) {
        collision_object = object;
    }
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
    void add_linked_object(Map_Object* obj) {
        if (obj == this) return;
        linked_objects.push_back(obj);
    }
    void remove_linked_object(Map_Object* obj) {
        linked_objects.erase(
            std::remove_if(linked_objects.begin(), linked_objects.end(),
                [obj](const Map_Object* o) { return o == obj; }),
            linked_objects.end());
    }
    bool is_outlined() const;
    void set_outlined(std::optional<bool> new_outlined) {
        outlined = new_outlined;
    }
    std::optional<xd::vec4> get_outline_color() const {
        return outline_color;
    }
    void set_outline_color(std::optional<xd::vec4> new_color) {
        outline_color = new_color;
    }
    Draw_Order get_draw_order() const {
        return draw_order;
    }
    void set_draw_order(Draw_Order order) {
        draw_order = order;
    }
    Sprite* get_sprite() override {
        return sprite.get();
    }
    const Sprite* get_sprite() const {
        return sprite.get();
    }
    void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") override;
    float get_speed() const {
        return speed;
    }
    void set_speed(float speed);
    // Update object's state
    void update_state(const std::string& new_state) {
        if (frozen) return;
        set_pose("", new_state);
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
    void set_pose(const std::string& new_pose_name = "", const std::string& new_state = "",
            Direction new_direction = Direction::NONE) {
        if (!new_pose_name.empty())
            pose_name = new_pose_name;
        if (!new_state.empty())
            state = new_state;
        if (new_direction != Direction::NONE)
            direction = new_direction;
        Sprite_Holder::set_pose(pose_name, state, direction);
        for (auto obj : linked_objects) {
            obj->set_pose(new_pose_name, new_state, new_direction);
        }
    }
    // Face another object
    void face(const Map_Object& other);
    // Face a certain spot
    void face(float x, float y);
    void face(xd::vec2 other_position);
    // Face a certain direction
    void face(Direction dir);
    // Run the object's activation script
    void run_trigger_script() {
        run_script(trigger_script);
    }
    // Run the touch script
    void run_touch_script() {
        run_script(touch_script);
    }
    // Run leave/exit script
    void run_leave_script() {
        run_script(leave_script);
    }
    // Serialize object to TMX data
    rapidxml::xml_node<>* save(rapidxml::xml_document<>& doc);
    // Load the object from TMX data
    static std::unique_ptr<Map_Object> load(rapidxml::xml_node<>& node, Game& game);
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
    // Object's display size, bounding box is not affected
    xd::vec2 magnification;
    // Outline color
    std::optional<xd::vec4> outline_color;
    // Is the object outlined?
    std::optional<bool> outlined;
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
    // How does passthrough work? INITIATOR can pass through others,
    // RECEIVER can be passed through. Defaults to BOTH 
    Passthrough_Type passthrough_type;
    // Does this object override tile collision?
    bool override_tile_collision;
    // Object direction
    Direction direction;
    // Current pose name
    std::string pose_name;
    // Current pose state
    std::string state;
    // Name of facing state
    std::string face_state;
    // Name of walking state
    std::string walk_state;
    // Context in which script is executed: globally or per map
    Script_Context script_context;
    // Script executed when object is triggered by pressing a button
    std::string trigger_script;
    // Script executed when the object is touched/collided with
    std::string touch_script;
    // Script executed when leaving/no longer touching this object
    std::string leave_script;
    // Should the object face player when triggered?
    bool player_facing;
    // Object currently colliding with player
    Map_Object* collision_object;
    // Area the plaer is inside
    Map_Object* collision_area;
    // Last object activated by player
    Map_Object* triggered_object;
    // Objects that move whenever this object moves
    std::vector<Map_Object*> linked_objects;
    // Object properties
    Tmx_Properties properties;
    // How object is drawn relative to other objects in layer
    Draw_Order draw_order;
    // Optional sprite representing the object
    std::shared_ptr<Sprite> sprite;
    // Object speed
    float speed;
    // Run a script
    void run_script(const std::string& script);
};

#endif
