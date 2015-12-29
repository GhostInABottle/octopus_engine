#ifndef HPP_CANVAS
#define HPP_CANVAS

#include <string>
#include <vector>
#include <xd/graphics/types.hpp>
#include <xd/graphics/texture.hpp>
#include <xd/graphics/font.hpp>
#include <xd/graphics/text_formatter.hpp>
#include "sprite_holder.hpp"
#include "sprite.hpp"

class Game;
namespace xd {
    class simple_text_renderer;
}

class Canvas : public Sprite_Holder {
public:
    // Create a canvas from a sprite
    Canvas(Game& game, const std::string& sprite, const std::string& pose_name, xd::vec2 position);
    // Create a canvas from an image file name
    Canvas(const std::string& filename, xd::vec2 position);
    // Create a canvas with a transparent color
    Canvas(const std::string& filename, xd::vec2 position, xd::vec4 trans);
    // Create a canvas with some text
    Canvas(Game& game, xd::vec2 position, const std::string& text);
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
    xd::vec2 get_origin() const {
        return origin;
    }
    void set_origin(xd::vec2 origin) {
        this->origin = origin;
    }
    xd::vec2 get_magnification() const {
        return magnification;
    }
    void set_magnification(xd::vec2 magnification) {
        this->magnification = magnification;
    }
    float get_angle() const {
        return angle;
    }
    void set_angle(float angle) {
        this->angle = angle;
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
    std::string get_filename() const {
        return filename;
    }
    xd::texture* get_texture() {
        return image_texture.get();
    }
    std::vector<std::string>& get_text_lines() {
        return text_lines;
    }
    xd::font_style* get_style() const {
        return style.get();
    }
private:
    // Canvas position
    xd::vec2 position;
    // Drawing origin
    xd::vec2 origin;
    // X and Y magnification
    xd::vec2 magnification;
    // Rotation angle in degrees
    float angle;
    // Opacity (0 transparent, 1 opaque)
    float opacity;
    // Is the canvas visible?
    bool visible;
    // Image filename
    std::string filename;
    // Image texture
    xd::texture::ptr image_texture;
    // Optional sprite
    Sprite::ptr sprite;
    // Text to print
    std::string text;
    // Text lines
    std::vector<std::string> text_lines;
    // Font to use
    xd::font* font;
    // Text renderer
    xd::simple_text_renderer* text_renderer;
    // Text style
    std::unique_ptr<xd::font_style> style;
    // Text formatter
    xd::text_formatter::ptr formatter;
};

#endif
