#ifndef HPP_BASE_CANVAS
#define HPP_BASE_CANVAS

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <optional>
#include "../xd/graphics/types.hpp"
#include "../xd/graphics/texture.hpp"
#include "../scripting/lua_object.hpp"
#include "../translucent_object.hpp"

class Game;
class Camera;

namespace xd {
    class sprite_batch;
}

class Base_Canvas : public Translucent_Object {
public:
    enum class Type { IMAGE, SPRITE, TEXT, MIXED };
    Base_Canvas(const Base_Canvas&) = delete;
    Base_Canvas& operator=(const Base_Canvas&) = delete;
    // Base class destructor
    virtual ~Base_Canvas() noexcept {}
    // Update or render canvas
    virtual void render(Camera& camera, xd::sprite_batch& batch, Base_Canvas* parent) = 0;
    virtual void update() {}
    // Add a new child canvas, forwards the arguments to the child Canvas constructor
    template<class CType, class ...Args>
    CType* add_child(const std::string& child_name, Args&&... args) {
        children.emplace_back(std::make_unique<CType>(std::forward<Args>(args)...));

        auto& child = children.back();
        child->parent = this;
        auto root_parent = this->root_parent ? this->root_parent : this;
        child->root_parent = root_parent;
        child->set_name(child_name);
        if (child->get_type() != children_type) {
            children_type = Type::MIXED;
        }
        child->set_priority(get_priority() + children.size());
        child->inherit_properties(*this);

        if (type != Type::TEXT && !fbo_texture) {
            setup_fbo();
        }

        root_parent->children_by_id[child->id] = child.get();

        redraw_needed = true;
        return static_cast<CType*>(child.get());
    }
    // Remove a child
    void remove_child(const std::string& name);
    // Inherit certain properties from another canvas
    virtual void inherit_properties(const Base_Canvas& parent) = 0;
    // Find a child by name
    Base_Canvas* get_child_by_name(const std::string& child_name) {
        auto child = std::find_if(children.begin(), children.end(),
            [&](auto& child) { return child->name == child_name; });
        return child != children.end() ? child->get() : nullptr;
    }
    // Find a child by ID
    Base_Canvas* get_child_by_id(int id) {
        auto child = children_by_id.find(id);
        if (child != children_by_id.end()) {
            return child->second;
        }

        if (!parent) return nullptr;

        // Manually look for ID if we're looking for the child of a child
        auto direct_child = std::find_if(children.begin(), children.end(),
            [&](auto& child) { return child->id == id; });
        return direct_child != children.end() ? direct_child->get() : nullptr;
    }
    // Find a child by index
    Base_Canvas* get_child_by_index(std::size_t index) {
        if (index >= 0 && index < children.size()) {
            return children[index].get();
        }
        return nullptr;
    }
    // Get number of children
    std::size_t get_child_count() const {
        return children.size();
    }
    // Setup FBO texture
    void setup_fbo();
    // Reset the canvas IDs
    static void reset_last_child_id() {
        last_canvas_id = -1;
    }
    // Getters and setters
    Base_Canvas* get_root_parent() {
        return root_parent;
    }
    Base_Canvas* get_parent() {
        return parent;
    }
    Lua_Object& get_lua_data() {
        return lua_data;
    }
    int get_id() const {
        return id;
    }
    std::string get_name() const {
        return name;
    }
    void set_name(const std::string& new_name) {
        name = new_name;
    }
    int get_priority() const {
        return priority;
    }
    void set_priority(int new_priority) {
        priority = new_priority;
    }
    xd::vec2 get_position() const {
        return position;
    }
    void set_position(xd::vec2 new_position) {
        if (position == new_position)
            return;
        position = new_position;
        redraw_needed = true;
    }
    float get_x() const {
        return position.x;
    }
    void set_x(float x) {
        if (position.x == x)
            return;
        position.x = x;
        redraw_needed = true;
    }
    float get_y() const {
        return position.y;
    }
    void set_y(float y) {
        if (position.y == y)
            return;
        position.y = y;
        redraw_needed = true;
    }
    xd::rect get_scissor_box() const {
        return scissor_box;
    }
    void set_scissor_box(xd::rect new_scissor_box) {
        if (scissor_box.x == new_scissor_box.x
            && scissor_box.y == new_scissor_box.y
            && scissor_box.w == new_scissor_box.w
            && scissor_box.h == new_scissor_box.h)
            return;
        scissor_box = new_scissor_box;
        redraw_needed = true;
    }
    float get_opacity() const override {
        return color.a;
    }
    void set_opacity(float opacity) override {
        if (color.a == opacity) return;

        color.a = opacity;
        redraw_needed = true;
    }
    virtual xd::vec4 get_color() const {
        return color;
    }
    virtual void set_color(xd::vec4 new_color) {
        if (color == new_color) return;

        color = new_color;
        redraw_needed = true;
    }
    bool is_visible() const {
        return visible;
    }
    void set_visible(bool new_visible) {
        if (visible == new_visible)
            return;
        visible = new_visible;
        redraw_needed = true;
    }
    bool should_update() const;
    std::shared_ptr<xd::texture> get_fbo_texture() const {
        return fbo_texture;
    }
    bool is_camera_relative() const {
        return camera_relative;
    }
    void set_camera_relative(bool new_value) {
        camera_relative = new_value;
    }
    Base_Canvas::Type get_type() const {
        return type;
    }
    Base_Canvas::Type get_children_type() const {
        return children_type;
    }
    bool should_redraw(int time) const;
    void redraw() {
        redraw_needed = true;
    }
    void mark_as_drawn(int time);
    bool has_background() const {
        return background_visible;
    }
    void set_background_visible(bool visible) {
        background_visible = visible;
    }
    xd::rect get_background_rect() const {
        return background_rect;
    }
    void set_background_rect(xd::rect new_rect) {
        background_rect = new_rect;
    }
    xd::vec4 get_background_color() const {
        return background_color;
    }
    void set_background_color(xd::vec4 new_color) {
        background_color = new_color;
    }
    bool is_paused_game_canvas() const {
        return paused_game_canvas;
    }
    xd::vec2 get_last_camera_position() const {
        return last_camera_position;
    }
    void set_last_camera_position(xd::vec2 new_pos) {
        last_camera_position = new_pos;
    }
protected:
    // Base class constructor
    Base_Canvas(Game& game, Base_Canvas::Type type, xd::vec2 position);
    // The game instance
    Game& game;
private:
    // The top-most parent of this canvas
    Base_Canvas* root_parent;
    // The direct parent of this canvas
    Base_Canvas* parent;
    // Allows defining extra Lua properties on the object
    Lua_Object lua_data;
    // Unique incremental ID
    int id;
    // Optional name used to identify the canvas
    std::string name;
    // Canvas priority, higher priority canvases are drawn on top
    int priority;
    // Type of Canvas content
    Type type;
    // Type of Canvas children
    Type children_type;
    // Canvas position
    xd::vec2 position;
    // Scissor test rectangle (won't draw outside it)
    xd::rect scissor_box;
    // Color applied to the canvas
    xd::vec4 color;
    // Is the canvas visible?
    bool visible;
    // Used when FBO is supported for faster rendering
    std::shared_ptr<xd::texture> fbo_texture;
    // Render relative to camera? (true by default)
    bool camera_relative;
    // List of child canvases that are rendered with this one
    std::vector<std::unique_ptr<Base_Canvas>> children;
    // Lookup children by unique ID (only stored at root parent)
    std::unordered_map<int, Base_Canvas*> children_by_id;
    // Did something change that requires the canvas to be redrawn?
    bool redraw_needed;
    // When was the last time the canvas was redrawn
    int last_drawn_time;
    // Camera position the last time the canvas was drawn (used to correct FBO position)
    xd::vec2 last_camera_position;
    // Opaque background to draw behind canvas
    bool background_visible;
    xd::rect background_rect;
    xd::vec4 background_color;
    // Was canvas created when game was paused
    bool paused_game_canvas;
    // The ID of the most recently created canvas
    inline static int last_canvas_id = -1;
};

#endif
