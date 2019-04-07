#ifndef HPP_CANVAS
#define HPP_CANVAS

#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "xd/graphics/types.hpp"
#include "xd/graphics/texture.hpp"
#include "xd/graphics/font.hpp"
#include "xd/graphics/text_formatter.hpp"
#include "xd/graphics/framebuffer.hpp"
#include "sprite_holder.hpp"
#include "sprite.hpp"
#include "text_parser.hpp"

class Game;
namespace xd {
    class simple_text_renderer;
}

class Canvas : public Sprite_Holder {
public:
    enum class Type { IMAGE, SPRITE, TEXT, MIXED };
    // Create a canvas from a sprite
    Canvas(Game& game, const std::string& sprite, const std::string& pose_name, xd::vec2 position);
    // Create an image canvas with an optional transparent color
    Canvas(const std::string& filename, xd::vec2 position, xd::vec4 trans = xd::vec4(0));
    // Create a canvas with some text
    Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative = true);
    // Add a new child canvas, forwards the arguments to the child Canvas constructor
    template<class ...Args>
    Canvas* add_child(const std::string& name, Args&&... args) {
        children.emplace_back(new Canvas(std::forward<Args>(args)...));
        auto& child = children.back();
        child->set_name(name);
        if (child->get_type() != children_type) {
            children_type = Type::MIXED;
        }
        child->set_priority(get_priority() + children.size());
        child->inherit_properties(*this);

        redraw_needed = true;
        return children.back().get();
    }
    // Remove a child
    void remove_child(const std::string& name);
    // Inherit certain properties from another canvas
    void inherit_properties(const Canvas& parent);
    // Find a child by name
    Canvas* get_child(const std::string& name) {
        auto child = std::find_if(children.begin(), children.end(),
            [&](auto& child) { return child->name == name; });
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
    void set_image(const std::string& filename, xd::vec4 trans = xd::vec4(0));
    // Update the sprite
    void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") override;
    // Update the text for a text canvas
    void set_text(const std::string& text);
    // Get the current canvas text
    std::string get_text() const { return text; }
    // Render canvas text
    void render_text(const std::string& text , float x, float y);
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
    void set_name(const std::string& name) {
        this->name = name;
    }
    int get_priority() const {
        return priority;
    }
    void set_priority(int priority) {
        this->priority = priority;
    }
    xd::vec2 get_position() const {
        return position;
    }
    void set_position(xd::vec2 position) {
        if (this->position == position)
            return;
        this->position = position;
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
    xd::vec2 get_origin() const {
        return origin;
    }
    void set_origin(xd::vec2 origin) {
        if (this->origin == origin)
            return;
        this->origin = origin;
        redraw_needed = true;
    }
    xd::vec2 get_magnification() const {
        return magnification;
    }
    void set_magnification(xd::vec2 magnification) {
        if (this->magnification == magnification)
            return;
        this->magnification = magnification;
        redraw_needed = true;
    }
    xd::rect get_scissor_box() const {
        return scissor_box;
    }
    void set_scissor_box(xd::rect scissor_box) {
        if (this->scissor_box.x == scissor_box.x
                && this->scissor_box.y == scissor_box.y
                && this->scissor_box.w == scissor_box.w
                && this->scissor_box.h == scissor_box.h)
            return;
        this->scissor_box = scissor_box;
        redraw_needed = true;
    }
    float get_angle() const {
        return angle;
    }
    void set_angle(float angle) {
        if (this->angle == angle)
            return;
        this->angle = angle;
        redraw_needed = true;
    }
    float get_opacity() const {
        return color.a;
    }
    void set_opacity(float opacity) {
        if (this->color.a == opacity)
            return;
        this->color.a = opacity;
        redraw_needed = true;
    }
    xd::vec4 get_color() const {
        return color;
    }
    void set_color(xd::vec4 color) {
        if (this->color == color)
            return;
        this->color = color;
        redraw_needed = true;
    }
    bool is_visible() const {
        return visible;
    }
    void set_visible(bool visible) {
        if (this->visible == visible)
            return;
        this->visible = visible;
        redraw_needed = true;
    }
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
    void set_text_color(xd::vec4 color) {
        if (style->color() == color)
            return;
        style->color() = color;
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
    bool has_outline() const {
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
    void set_text_shadow_color(xd::vec4 color) {
        if (!style->has_shadow())
            style->shadow(1.0, 1.0, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (style->shadow().color == color)
            return;
        style->shadow().color = color;
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
    void set_text_type(const std::string& type) {
        if (style->has_type() && style->type() == type)
            return;
        style->type(type);
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
    bool should_redraw(int time) const;
    void mark_as_drawn(int time);
private:
    // Sets shared default values
    Canvas(xd::vec2 position);
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
    xd::vec2 origin;
    // X and Y magnification
    xd::vec2 magnification;
    // Scissor test rectangle (won't draw outside it)
    xd::rect scissor_box;
    // Rotation angle in degrees
    float angle;
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
    // Text renderer
    xd::simple_text_renderer* text_renderer;
    // Text style
    std::unique_ptr<xd::font_style> style;
    // Text formatter
    std::unique_ptr<xd::text_formatter> formatter;
    // Render relative to camera? (by default: false for text, true otherwise)
    bool camera_relative;
    // List of child canvases that are rendered with this one
    std::vector<std::unique_ptr<Canvas>> children;
    // Did something change that requires the canvas to be redrawn?
    bool redraw_needed;
    // When was the last time the canvas was redrawn
    int last_drawn_time;
    // Used for parsing text tags
    Text_Parser parser;
    // Ignore missing tags
    bool permissive_tag_parsing;
};

#endif
