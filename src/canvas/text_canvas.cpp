#include "text_canvas.hpp"
#include "base_canvas.hpp"
#include "../camera.hpp"
#include "../decorators/typewriter_decorator.hpp"
#include "../game.hpp"
#include "../text_parser.hpp"
#include "../xd/glm.hpp"
#include "../xd/graphics/font.hpp"
#include "../xd/graphics/font_style.hpp"
#include "../xd/graphics/sprite_batch.hpp"
#include <iosfwd>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

Text_Canvas::Text_Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative,
    std::optional<Typewriter_Options> typewriter_options, bool is_child)
        : Base_Canvas(game, Base_Canvas::Type::TEXT, position),
        centered(false),
        permissive_tag_parsing(false),
        typewriter_options(typewriter_options),
        typewriter_line(0) {
    font = game.get_font();
    style = std::make_unique<xd::font_style>(game.get_font_style());
    set_camera_relative(camera_relative);

    if (!is_child) {
        setup_fbo();
    }

    set_text(text);
}

void Text_Canvas::inherit_properties(const Base_Canvas& parent) {
    Base_Canvas::inherit_properties(parent);

    auto text_parent = dynamic_cast<const Text_Canvas*>(&parent);
    if (!text_parent) return;

    set_font(text_parent->get_font_filename());
    set_font_size(text_parent->get_font_size());

    if (text_parent->has_font_type()) {
        set_font_type(text_parent->get_font_type());
    }

    if (text_parent->has_outline()) {
        set_outline_color(text_parent->get_outline_color());
        set_outline_width(text_parent->get_outline_width());
    }

    if (text_parent->has_shadow()) {
        set_shadow_color(text_parent->get_shadow_color());
        set_shadow_offset(text_parent->get_shadow_offset());
    }
}

void Text_Canvas::set_text(const std::string& new_text) {
    if (text == new_text && !text.empty()) return;

    text = new_text;
    text_lines = Text_Parser::split_to_lines(new_text, permissive_tag_parsing);
    if (centered) {
        line_widths = calculate_line_widths(game, text_lines, style.get());
    }
    redraw();
}

float Text_Canvas::get_text_width(const std::string& text) {
    return game.text_width(text, font.get(), style.get());
}

const std::string Text_Canvas::get_font_filename() const {
    return font->filename();
}

void Text_Canvas::set_font(const std::string& font_file) {
    if (font_file == get_font_filename()) return;

    font = game.create_font(font_file);
    redraw();
}

void Text_Canvas::link_font(const std::string& font_type, const std::string& font_file) {
    font->link_font(font_type, game.create_font(font_file));
}

void Text_Canvas::render(Camera& camera, xd::sprite_batch&, Base_Canvas* parent) {
    style->color().a = get_opacity();
    auto& lines = get_lines();
    xd::vec2 pos = get_position();
    auto camera_pos = camera.get_pixel_position();

    if (parent) {
        pos += parent->get_position();
    }

    for (auto i = 0U; i < lines.size(); ++i) {
        auto& line = lines[i];
        float draw_x = pos.x;
        float draw_y = pos.y;
        if (!is_camera_relative()) {
            draw_x -= camera_pos.x;
            draw_y -= camera_pos.y;
        }

        if (centered) {
            auto width = line_widths.at(i);
            draw_x -= width / 2;
        }

        render_text(line, draw_x, draw_y, i);
        pos.y += style->line_height();
    }
}

void Text_Canvas::skip_typewriter() {
    if (!typewriter_options) return;
    typewriter_line = text_lines.size();
    auto& decorator = game.get_typewriter_decorator();
    decorator.reset_slot(typewriter_options->slot);
}

void Text_Canvas::render_text(const std::string& text_to_render, float x, float y, unsigned int line_number) {
    auto apply_typewriter = typewriter_options && !typewriter_done();
    if (apply_typewriter && line_number > typewriter_line) {
        // Still showing a previous line using the typewriter effect
        return;
    } else if (!apply_typewriter || line_number < typewriter_line) {
        game.render_text(*font, *style, x, y, text_to_render);
        return;
    }

    auto slot = typewriter_options->slot;
    auto& decorator = game.get_typewriter_decorator();
    if (decorator.is_done(slot)) {
        apply_typewriter = false;
        decorator.reset_slot(slot);
        if (typewriter_line < text_lines.size()) {
            ++typewriter_line;
        }
    }

    std::string text;
    if (apply_typewriter) {
        std::stringstream ss;
        ss << "{typewriter=" << slot << ","
           << typewriter_options->delay << ","
           << typewriter_options->sound_filename << ","
           << typewriter_options->sound_volume << ","
           << typewriter_options->sound_pitch << ","
           << typewriter_options->sound_max_pitch << "}"
           << text_to_render << "{/typewriter}";
        text = ss.str();
    }

    game.render_text(*font, *style, x, y, apply_typewriter ? text : text_to_render);
    return;
}

std::vector<float> Text_Canvas::calculate_line_widths(Game& game,
        const std::vector<std::string>& lines, xd::font_style* style) {
    std::vector<float> widths;

    for (auto& line : lines) {
        widths.push_back(game.text_width(line, nullptr, style));
    }

    return widths;
}
