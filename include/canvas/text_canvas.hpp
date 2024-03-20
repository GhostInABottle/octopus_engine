#ifndef HPP_TEXT_CANVAS
#define HPP_TEXT_CANVAS

#include "../xd/graphics/font_style.hpp"
#include "base_canvas.hpp"
#include <optional>

namespace xd {
    class simple_text_renderer;
    class font;
}

// A canvas for displaying tet
class Text_Canvas : public Base_Canvas {
public:
    struct Typewriter_Options {
        // Slot used by the typewriter effect
        int slot{-1};
        // The delay for showing each character
        int delay{-1};
        // The sound effect to play when showing each character
        std::string sound_filename;
        // The sound volume
        float sound_volume{1.0f};
        // The sound pitch
        float sound_pitch{1.0f};
        // If specified, a random value between pitch and max will be used
        float sound_max_pitch{-1.0f};
    };
    Text_Canvas(const Text_Canvas&) = delete;
    Text_Canvas& operator=(const Text_Canvas&) = delete;
    // Create a canvas with some text
    Text_Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative = true,
        std::optional<Typewriter_Options> typewriter_options = std::nullopt, bool is_child = false);
    // Inherit certain properties from another canvas
    void inherit_properties(const Base_Canvas& parent) override;
    // Update the text for a text canvas
    void set_text(const std::string& text);
    // Get the current canvas text
    std::string get_text() const { return text; }
    // Get the individual text lines
    std::vector<std::string>& get_lines() {
        return text_lines;
    }
    // Calculate the pixel width of some text
    float get_text_width(const std::string& text);
    // Get the font style
    xd::font_style* get_style() {
        return style.get();
    }
    // Get the name of the font file
    const std::string get_font_filename() const;
    // Get and set the font size
    int get_font_size() const {
        return style->size();
    }
    void set_font_size(int size) {
        if (style->size() == size) return;

        style->size() = size;
        redraw();
    }
    // Get and set the text color
    xd::vec4 get_color() const override {
        return style->color();
    }
    void set_color(xd::vec4 text_color) override {
        if (style->color() == text_color) return;

        style->color() = text_color;
        redraw();
    }
    // Get and set the pixel height of each line
    float get_line_height() const {
        return style->line_height();
    }
    void set_line_height(float height) {
        if (style->line_height() == height)
            return;
        style->line_height() = height;
        redraw();
    }
    // Get and set the size of the text outline
    int get_outline_width() const {
        if (!style->has_outline()) return 0;
        return style->outline().width;
    }
    void set_outline_width(int width) {
        if (!style->has_outline()) {
            style->outline(1, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }

        if (style->outline().width == width) return;

        style->outline().width = width;
        redraw();
    }
    // Get and set the color of the text outline
    xd::vec4 get_outline_color() const {
        if (!style->has_outline()) return xd::vec4();

        return style->outline().color;
    }
    void set_outline_color(xd::vec4 color) {
        if (!style->has_outline()) {
            style->outline(1, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }

        if (style->outline().color == color) return;
        style->outline().color = color;
        redraw();
    }
    // Check if text is drawn with an outline
    bool has_outline() const {
        return style->has_outline();
    }
    // Remove any outline
    void reset_outline() {
        style->reset_outline();
        redraw();
    }
    // Get and set the offset of the text shadow
    xd::vec2 get_shadow_offset() const {
        if (!style->has_shadow()) return xd::vec2();
        return xd::vec2(style->shadow().x, style->shadow().y);
    }
    void set_shadow_offset(xd::vec2 offset) {
        if (!style->has_shadow()) {
            style->shadow(1.0, 1.0, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }

        auto& shadow = style->shadow();
        if (shadow.x == offset.x && shadow.y == offset.y) return;

        shadow.x = offset.x;
        shadow.y = offset.y;
        redraw();
    }
    // Get and set the color of the text shadow
    xd::vec4 get_shadow_color() const {
        if (!style->has_shadow()) return xd::vec4();
        return style->shadow().color;
    }
    void set_shadow_color(xd::vec4 shadow_color) {
        if (!style->has_shadow()) {
            style->shadow(1.0, 1.0, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }

        if (style->shadow().color == shadow_color) return;

        style->shadow().color = shadow_color;
        redraw();
    }
    // Check if text is drawn with a shadow
    bool has_shadow() const {
        return style->has_shadow();
    }
    // Remove the text shadow
    void reset_shadow() {
        style->reset_shadow();
        redraw();
    }
    // Get and set the type (bold/italic/etc) of the font
    std::string get_font_type() const {
        if (!style->has_type()) return "";
        return style->type();
    }
    void set_font_type(const std::string& text_type) {
        if (style->has_type() && style->type() == text_type)
            return;
        style->type(text_type);
        redraw();
    }
    // Check if text has a type applied
    bool has_font_type() const {
        return style->has_type();
    }
    // Remove any applied font type
    void reset_font_type() {
        style->reset_type();
        redraw();
    }
    // Get and set whether text parsing errors are ignored
    bool has_permissive_tag_parsing() const {
        return permissive_tag_parsing;
    }
    void set_permissive_tag_parsing(bool value) {
        permissive_tag_parsing = value;
    }
    // Set the font
    void set_font(const std::string& font_file);
    // Set bold and italic fonts
    void link_font(const std::string& type, const std::string& font_file);
    // Render the text
    void render(Camera& camera, xd::sprite_batch& batch, Base_Canvas* parent) override;
    // Check if the typewriter effect is done
    bool typewriter_done() const {
        return !typewriter_options || typewriter_line >= text_lines.size();
    }
    // Show the whole text without the typewriter effect
    void skip_typewriter();
private:
    // Text to print
    std::string text;
    // Text lines
    std::vector<std::string> text_lines;
    // Font to use
    std::shared_ptr<xd::font> font;
    // Text style
    std::unique_ptr<xd::font_style> style;
    // Ignore missing tags
    bool permissive_tag_parsing;
    // Options for the typewriter effect
    std::optional<Typewriter_Options> typewriter_options;
    // Index of current line being printed with the typewriter effect
    unsigned int typewriter_line;
    // Render text at a position
    void render_text(const std::string& text, float x, float y, unsigned int line_number);
};

#endif