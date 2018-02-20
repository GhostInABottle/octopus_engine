#ifndef HPP_CANVAS
#define HPP_CANVAS

#include <string>
#include <vector>
#include <algorithm>
#include <xd/graphics/types.hpp>
#include <xd/graphics/texture.hpp>
#include <xd/graphics/font.hpp>
#include <xd/graphics/text_formatter.hpp>
#include <xd/graphics/framebuffer.hpp>
#include "sprite_holder.hpp"
#include "sprite.hpp"

class Game;
namespace xd {
    class simple_text_renderer;
}

class Canvas : public Sprite_Holder {
public:
    enum class Type { IMAGE, SPRITE, TEXT };
    // Create a canvas from a sprite
    Canvas(Game& game, const std::string& sprite, const std::string& pose_name, xd::vec2 position);
    // Create a canvas from an image file name
    Canvas(const std::string& filename, xd::vec2 position);
    // Create a canvas with a transparent color
    Canvas(const std::string& filename, xd::vec2 position, xd::vec4 trans);
    // Create a canvas with some text
    Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative = true);
    // Add a new child canvas, forwards the arguments to the child Canvas constructor
    template<class ...Args>
    void add_child(const std::string& name, Args... args);
    // Remove a child
    void remove_child(const std::string& name);
    // Find a child by name
    Canvas* get_child(const std::string& name) {
        for (auto& child : children) {
            if (child->name == name)
                return child.get();
        }
        return nullptr;
    }
    // Find a child by index
    Canvas* get_child(std::size_t index) {
        if (index >= 0 && index < children.size()) {
            return children[index].get();
        }
        return nullptr;
    }
    // Get number of children
    std::size_t child_count() const {
        return children.size();
    }
    // Setup FBO texture
    void setup_fbo();
    // Update the image
    void set_image(const std::string& filename, xd::vec4 trans = xd::vec4(0));
    // Update the sprite
    void set_sprite(Game& game, xd::asset_manager& manager, const std::string& filename, const std::string& pose_name = "");
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
    Sprite* get_sprite() { return sprite.get(); }
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
    xd::vec2 get_position() const {
        return position;
    }
    void set_position(xd::vec2 position) {
        this->position = position;
        redraw_needed = true;
    }
    float get_x() const {
        return position.x;
    }
    void set_x(float x) {
        position.x = x;
        redraw_needed = true;
    }
    float get_y() const {
        return position.y;
    }
    void set_y(float y) {
        position.y = y;
        redraw_needed = true;
    }
    xd::vec2 get_origin() const {
        return origin;
    }
    void set_origin(xd::vec2 origin) {
        this->origin = origin;
        redraw_needed = true;
    }
    xd::vec2 get_magnification() const {
        return magnification;
    }
    void set_magnification(xd::vec2 magnification) {
        this->magnification = magnification;
        redraw_needed = true;
    }
    xd::rect get_scissor_box() const {
        return scissor_box;
    }
    void set_scissor_box(xd::rect scissor_box) {
        this->scissor_box = scissor_box;
        redraw_needed = true;
    }
    float get_angle() const {
        return angle;
    }
    void set_angle(float angle) {
        this->angle = angle;
        redraw_needed = true;
    }
    float get_opacity() const {
        return color.a;
    }
    void set_opacity(float opacity) {
        this->color.a = opacity;
        redraw_needed = true;
    }
    xd::vec4 get_color() const {
        return color;
    }
    void set_color(xd::vec4 color) {
        this->color = color;
        redraw_needed = true;
    }
    bool is_visible() const {
        return visible;
    }
    void set_visible(bool visible) {
        this->visible = visible;
    }
    std::string get_filename() const {
        return filename;
    }
    xd::texture::ptr get_image_texture() const {
        return image_texture;
    }
    xd::texture::ptr get_fbo_texture() const {
        return fbo_texture;
    }
    const xd::framebuffer& get_framebuffer() const {
        return framebuffer;
    }
    std::vector<std::string>& get_text_lines() {
        return text_lines;
    }
    xd::font_style* get_style() const {
        return style.get();
    }
    bool is_camera_relative_text() const {
        return camera_relative_text;
    }
    int get_font_size() const {
        return style->size();
    }
    void set_font_size(int size) {
        style->size() = size;
        redraw_needed = true;
    }
    xd::vec4 get_text_color() const {
        return style->color();
    }
    void set_text_color(xd::vec4 color) {
        style->color() = color;
        redraw_needed = true;
    }
    float get_line_height() const {
        return style->line_height();
    }
    void set_line_height(float height) {
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
        style->outline().color = color;
        redraw_needed = true;
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
        style->shadow().color = color;
        redraw_needed = true;
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
        style->type(type);
        redraw_needed = true;
    }
    void reset_text_type() {
        style->reset_type();
        redraw_needed = true;
    }
    Canvas::Type get_type() const {
        return type;
    }
    bool should_redraw() const {
        bool redraw_children = std::any_of(std::begin(children), std::end(children),
            [](const std::unique_ptr<Canvas>& c) { return c->should_redraw();  });
        // Sprites and images are always redrawn, might change it later
        return redraw_needed || type != Canvas::Type::TEXT || redraw_children;
    }
    void mark_redrawn() {
        redraw_needed = false;
    }
private:
    // Optional name used to identify the canvas
    std::string name;
    // Type of Canvas content
    Type type;
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
    xd::texture::ptr image_texture;
    // Used when FBO is supported for faster rendering
    xd::texture::ptr fbo_texture;
    xd::framebuffer framebuffer;
    // Optional sprite
    Sprite::ptr sprite;
    // Text to print
    std::string text;
    // Text lines
    std::vector<std::string> text_lines;
    // Font to use
    xd::font::ptr font;
    // Text renderer
    xd::simple_text_renderer* text_renderer;
    // Text style
    std::unique_ptr<xd::font_style> style;
    // Text formatter
    xd::text_formatter::ptr formatter;
    // Is text canvas rendered relative to camera?
    bool camera_relative_text;
    // List of child canvases that are rendered with this one
    std::vector<std::unique_ptr<Canvas>> children;
    // Did something change that requires the canvas to be redrawn?
    bool redraw_needed;
};

#endif
