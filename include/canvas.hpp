#ifndef HPP_CANVAS
#define HPP_CANVAS

#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <utility>
#include <optional>
#include "xd/graphics/types.hpp"
#include "xd/graphics/texture.hpp"
#include "xd/graphics/font.hpp"
#include "xd/graphics/text_formatter.hpp"
#include "xd/graphics/framebuffer.hpp"
#include "sprite_holder.hpp"
#include "sprite.hpp"
#include "text_parser.hpp"
#include "lua_object.hpp"

class Game;
namespace xd {
    class simple_text_renderer;
}

class Canvas : public Sprite_Holder, public Lua_Object {
public:
    enum class Type { IMAGE, SPRITE, TEXT, MIXED };
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    // Create a canvas from a sprite
    Canvas(Game& game, const std::string& sprite, const std::string& pose_name, xd::vec2 position);
    // Create an image canvas with an optional transparent color
    Canvas(Game& game, const std::string& filename, xd::vec2 position, xd::vec4 trans = xd::vec4(0));
    // Create a canvas with some text
    Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative = true, bool is_child = false);
    // Add a new child canvas, forwards the arguments to the child Canvas constructor
    template<class ...Args>
    Canvas* add_child(const std::string& child_name, Args&&... args) {
        children.emplace_back(std::make_unique<Canvas>(std::forward<Args>(args)...));
        auto& child = children.back();
        child->set_name(child_name);
        if (child->get_type() != children_type) {
            children_type = Type::MIXED;
        }
        child->set_priority(get_priority() + children.size());
        child->inherit_properties(*this);
        if (type != Type::TEXT && !framebuffer) {
            setup_fbo();
        }

        redraw_needed = true;
        return children.back().get();
    }
    // Remove a child
    void remove_child(const std::string& name);
    // Inherit certain properties from another canvas
    void inherit_properties(const Canvas& parent);
    // Find a child by name
    Canvas* get_child(const std::string& child_name) {
        auto child = std::find_if(children.begin(), children.end(),
            [&](auto& child) { return child->name == child_name; });
        return child != children.end() ? child->get() : nullptr;
    }
    // Find a child by index
    Canvas* get_child(std::size_t index) {
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
    // Update the image
    void set_image(std::string filename, xd::vec4 trans = xd::vec4(0));
    // Update the sprite
    void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") override;
    // Get pose name
    std::string get_pose_name();
    // Get pose state
    std::string get_pose_state();
    // Get pose direction
    Direction get_pose_direction();
    // Update the text for a text canvas
    void set_text(const std::string& text);
    // Get the current canvas text
    std::string get_text() const { return text; }
    // Render canvas text
    void render_text(const std::string& text , float x, float y) const;
    // Set the font
    void set_font(const std::string& font_file);
    // Set bold and italic fonts
    void link_font(const std::string& type, const std::string& font_file);
    // Get the sprite, if any
    Sprite* get_sprite() override { return sprite.get(); }
    // Get canvas width
    int get_width() const {
        if (image_texture)
            return image_texture->width();
        return 0;
    }
    // Get canvas height
    int get_height() const {
        if (image_texture)
            return image_texture->height();
        return 0;
    }
    // Getters and setters
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
    std::optional<xd::vec2> get_origin() const {
        return origin;
    }
    void set_origin(std::optional<xd::vec2> new_origin) {
        if (origin == new_origin)
            return;
        origin = new_origin;
        redraw_needed = true;
    }
    xd::vec2 get_magnification() const {
        return magnification;
    }
    void set_magnification(xd::vec2 new_magnification) {
        if (magnification == new_magnification)
            return;
        magnification = new_magnification;
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
    std::optional<float> get_angle() const {
        return angle;
    }
    void set_angle(std::optional<float> new_angle) {
        if (angle == new_angle)
            return;
        angle = new_angle;
        redraw_needed = true;
    }
    float get_opacity() const {
        return color.a;
    }
    void set_opacity(float opacity) {
        if (color.a == opacity)
            return;
        color.a = opacity;
        redraw_needed = true;
    }
    xd::vec4 get_color() const {
        return color;
    }
    void set_color(xd::vec4 new_color) {
        if (color == new_color)
            return;
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
    std::string get_filename() const {
        return filename;
    }
    std::shared_ptr<xd::texture> get_image_texture() const {
        return image_texture;
    }
    std::shared_ptr<xd::texture> get_fbo_texture() const {
        return fbo_texture;
    }
    std::shared_ptr<xd::framebuffer> get_framebuffer() const {
        return framebuffer;
    }
    std::vector<std::string>& get_text_lines() {
        return text_lines;
    }
    float get_text_width(const std::string& text);
    xd::font_style* get_style() const {
        return style.get();
    }
    bool is_camera_relative() const {
        return camera_relative;
    }
    const std::string get_font_filename() const {
        return font->filename();
    }
    int get_font_size() const {
        return style->size();
    }
    void set_font_size(int size) {
        if (style->size() == size)
            return;
        style->size() = size;
        redraw_needed = true;
    }
    xd::vec4 get_text_color() const {
        return style->color();
    }
    void set_text_color(xd::vec4 text_color) {
        if (style->color() == text_color)
            return;
        style->color() = text_color;
        redraw_needed = true;
    }
    float get_line_height() const {
        return style->line_height();
    }
    void set_line_height(float height) {
        if (style->line_height() == height)
            return;
        style->line_height() = height;
        redraw_needed = true;
    }
    int get_text_outline_width() const {
        if (!style->has_outline())
            return 0;
        return style->outline().width;
    }
    void set_text_outline_width(int width) {
        if (!style->has_outline())
            style->outline(1, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (style->outline().width == width)
            return;
        style->outline().width = width;
        redraw_needed = true;
    }
    xd::vec4 get_text_outline_color() const {
        if (!style->has_outline())
            return xd::vec4();
        return style->outline().color;
    }
    void set_text_outline_color(xd::vec4 color) {
        if (!style->has_outline())
            style->outline(1, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (style->outline().color == color)
            return;
        style->outline().color = color;
        redraw_needed = true;
    }
    bool has_text_outline() const {
        return style->has_outline();
    }
    void reset_text_outline() {
        style->reset_outline();
        redraw_needed = true;
    }
    xd::vec2 get_text_shadow_offset() const {
        if (!style->has_shadow())
            return xd::vec2();
        return xd::vec2(style->shadow().x, style->shadow().y);
    }
    void set_text_shadow_offset(xd::vec2 offset) {
        if (!style->has_shadow())
            style->shadow(1.0, 1.0, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (style->shadow().x == offset.x && style->shadow().y == offset.y)
            return;
        style->shadow().x = offset.x;
        style->shadow().y = offset.y;
        redraw_needed = true;
    }
    xd::vec4 get_text_shadow_color() const {
        if (!style->has_shadow())
            return xd::vec4();
        return style->shadow().color;
    }
    void set_text_shadow_color(xd::vec4 shadow_color) {
        if (!style->has_shadow())
            style->shadow(1.0, 1.0, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (style->shadow().color == shadow_color)
            return;
        style->shadow().color = shadow_color;
        redraw_needed = true;
    }
    bool has_shadow() const {
        return style->has_shadow();
    }
    void reset_text_shadow() {
        style->reset_shadow();
        redraw_needed = true;
    }
    std::string get_text_type() const {
        if (!style->has_type())
            return std::string();
        return style->type();
    }
    void set_text_type(const std::string& text_type) {
        if (style->has_type() && style->type() == text_type)
            return;
        style->type(text_type);
        redraw_needed = true;
    }
    bool has_text_type() const {
        return style->has_type();
    }
    void reset_text_type() {
        style->reset_type();
        redraw_needed = true;
    }
    Canvas::Type get_type() const {
        return type;
    }
    Canvas::Type get_children_type() const {
        return children_type;
    }
    bool get_permissive_tag_parsing() const {
        return permissive_tag_parsing;
    }
    void set_permissive_tag_parsing(bool value) {
        permissive_tag_parsing = value;
    }
    xd::vec4 get_image_outline_color() const {
        return outline_color;
    }
    void set_image_outline_color(xd::vec4 color) {
        outline_color = color;
    }
    bool has_image_outline() const {
        return use_outline_shader;
    }
    void set_image_outline(bool value) {
        use_outline_shader = value;
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
private:
    // Sets shared default values
    Canvas(Game& game, xd::vec2 position);
    // The game instance
    Game& game;
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
    // Drawing origin
    std::optional<xd::vec2> origin;
    // X and Y magnification
    xd::vec2 magnification;
    // Scissor test rectangle (won't draw outside it)
    xd::rect scissor_box;
    // Rotation angle in degrees
    std::optional<float> angle;
    // Color applied to the image
    xd::vec4 color;
    // Is the canvas visible?
    bool visible;
    // Image filename
    std::string filename;
    // Image texture
    std::shared_ptr<xd::texture> image_texture;
    // Used when FBO is supported for faster rendering
    std::shared_ptr<xd::texture> fbo_texture;
    std::shared_ptr<xd::framebuffer> framebuffer;
    // Optional sprite
    std::unique_ptr<Sprite> sprite;
    // Text to print
    std::string text;
    // Text lines
    std::vector<std::string> text_lines;
    // Font to use
    std::shared_ptr<xd::font> font;
    // Text style
    std::unique_ptr<xd::font_style> style;
    // Render relative to camera? (by default: true for text, false otherwise)
    bool camera_relative;
    // List of child canvases that are rendered with this one
    std::vector<std::unique_ptr<Canvas>> children;
    // Did something change that requires the canvas to be redrawn?
    bool redraw_needed;
    // When was the last time the canvas was redrawn
    int last_drawn_time;
    // Camera position the last time the canvas was drawn (used to correct FBO position)
    xd::vec2 last_camera_position;
    // Ignore missing tags
    bool permissive_tag_parsing;
    // Apply outline to drawn images?
    bool use_outline_shader;
    // Color of image outline
    xd::vec4 outline_color;
    // Opaque background to draw behind canvas
    bool background_visible;
    xd::rect background_rect;
    xd::vec4 background_color;
    // Was canvas created when game was paused
    bool paused_game_canvas;
};

#endif
