#include "../../include/canvas/text_canvas.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/file.hpp"
#include "../../include/camera.hpp"
#include <stdexcept>

Text_Canvas::Text_Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative, bool is_child)
        : Base_Canvas(game, Base_Canvas::Type::TEXT, position),
        permissive_tag_parsing(false) {
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
    redraw();
}

float Text_Canvas::get_text_width(const std::string& text) {
    return game.text_width(text, font.get(), style.get());
}

void Text_Canvas::set_font(const std::string& font_file) {
    if (font_file == get_font_filename()) return;

    font = game.create_font(font_file);
    redraw();
}

void Text_Canvas::link_font(const std::string& font_type, const std::string& font_file) {
    if (!file_utilities::file_exists(font_file)) {
        throw std::runtime_error("Couldn't read font file " + font_file);
    }

    font->link_font(font_type, game.create_font(font_file));
}

void Text_Canvas::render(Camera& camera, xd::sprite_batch& batch, Base_Canvas* parent) {
    style->color().a = get_opacity();
    auto& lines = get_lines();
    xd::vec2 pos = get_position();
    auto camera_pos = camera.get_pixel_position();

    if (parent) {
        pos += parent->get_position();
    }

    for (const auto& line : lines) {
        float draw_x = pos.x;
        float draw_y = pos.y;
        if (!is_camera_relative()) {
            draw_x -= camera_pos.x;
            draw_y -= camera_pos.y;
        }

        render_text(line, draw_x, draw_y);
        pos.y += style->line_height();
    }
}

void Text_Canvas::render_text(const std::string& text_to_render, float x, float y) const {
    game.render_text(*font, *style, x, y, text_to_render);
}
