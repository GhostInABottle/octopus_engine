#include <cmath>
#include "../include/xd/graphics/stock_text_formatter.hpp"
#include "../include/xd/graphics/simple_text_renderer.hpp"
#include "../include/xd/graphics/framebuffer.hpp"
#include "../include/canvas.hpp"
#include "../include/utility.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/sprite_data.hpp"
#include "../include/shake_decorator.hpp"
#include "../include/configurations.hpp"
#include "../include/log.hpp"

Canvas::Canvas(xd::vec2 position) :
    priority(0), position(position), origin(0.5f, 0.5f), magnification(1.0f, 1.0f),
    angle(0.0f), color(1.0f), visible(false), redraw_needed(true),
    permissive_tag_parsing(false) {}

Canvas::Canvas(Game& game, const std::string& sprite, const std::string& pose_name, xd::vec2 position) : Canvas(position) {
    camera_relative = false;
    children_type = Canvas::Type::SPRITE;
    set_sprite(game, sprite, pose_name);
}

Canvas::Canvas(const std::string& filename, xd::vec2 position, xd::vec4 trans) : Canvas(position) {
    camera_relative = false;
    children_type = Canvas::Type::IMAGE;
    set_image(filename, trans);
}

Canvas::Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative) : Canvas(position) {
    text_renderer = &game.get_text_renderer();
    font = game.get_font();
    formatter = xd::create<xd::stock_text_formatter>();
    auto shake_decorator = game.get_shake_decorator();
    formatter->register_decorator("shake", [=](xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args) {
        shake_decorator->operator()(decorator, text, args);
    });
    style = std::make_unique<xd::font_style>(game.get_font_style());
    this->camera_relative = camera_relative;
    setup_fbo();
    type = Canvas::Type::TEXT;
    children_type = type;
    set_text(text);
}

void Canvas::setup_fbo() {
    if (Configurations::get<bool>("debug.use-fbo")
            && xd::framebuffer::extension_supported()) {
        int width = static_cast<int>(Configurations::get<float>("debug.width"));
        int height = static_cast<int>(Configurations::get<float>("debug.height"));
        framebuffer = std::make_unique<xd::framebuffer>();
        fbo_texture = xd::create<xd::texture>(width, height, nullptr,
            xd::vec4(0), GL_CLAMP, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }
}

void Canvas::set_image(const std::string& filename, xd::vec4 trans) {
    if (this->filename == filename)
        return;
    type = Canvas::Type::IMAGE;
    this->filename = filename;
    image_texture = xd::create<xd::texture>(normalize_slashes(filename),
        trans, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    redraw_needed = true;
}

void Canvas::set_sprite(Game& game, const std::string& filename, const std::string& pose_name) {
    if (this->filename == filename)
        return;
    type = Canvas::Type::SPRITE;
    this->filename = filename;
    sprite = std::make_unique<Sprite>(game,
        Sprite_Data::load(game.get_asset_manager(), filename));
    set_pose(pose_name, "", Direction::NONE);
    redraw_needed = true;
}

void Canvas::remove_child(const std::string& name) {
    children.erase(
        std::remove_if(children.begin(), children.end(),
            [&name](std::unique_ptr<Canvas>& c) { return c->get_name() == name; }
        ), children.end());
    // Update children type if needed
    if (!children.empty() && children_type == Type::MIXED) {
        auto child_type = children[0]->get_type();
        for (auto& c : children) {
            if (c->get_type() != child_type) {
                child_type = Type::MIXED;
                break;
            }
            children_type = child_type;
        }
    }
    redraw_needed = true;
}

void Canvas::inherit_properties(const Canvas& parent) {
    // Child inherits parent properties
    set_color(parent.get_color());
    set_angle(parent.get_angle());
    set_magnification(parent.get_magnification());
    set_origin(parent.get_origin());
    set_scissor_box(parent.get_scissor_box());

    if (parent.get_type() == Canvas::Type::TEXT && parent.get_type() == type) {
        set_font(parent.get_font_filename());
        set_font_size(parent.get_font_size());
        if (parent.has_text_type())
            set_text_type(parent.get_text_type());
        if (parent.has_outline()) {
            set_text_outline_color(parent.get_text_outline_color());
            set_text_outline_width(parent.get_text_outline_width());
        }
        if (parent.has_shadow()) {
            set_text_shadow_color(parent.get_text_shadow_color());
            set_text_shadow_offset(parent.get_text_shadow_offset());
        }
    }
}

void Canvas::set_text(const std::string& text) {
    if (this->text == text && !this->text.empty())
        return;
    this->text = text;
    redraw_needed = true;
    // Split tags across multiple lines
    // e.g. "{a=b}x\ny{/a}" => "{a=b}x{/a}", "{a=b}y{/a}"
    text_lines = split(text, "\n", false);
    if (text_lines.size() > 1 || permissive_tag_parsing) {
        std::string open_tags;
        for (auto& line : text_lines) {
            line = open_tags + line;
            open_tags = "";

            auto line_tokens = parser.parse(line, permissive_tag_parsing);

            for (auto i = line_tokens.rbegin(); i != line_tokens.rend(); i++) {
                auto& token = *i;
                if (token.unmatched && token.type == "opening_tag") {
                    // Close open tag and remember it for following lines
                    line += "{/" + token.tag + "}";
                    auto tag = "{" + token.tag;
                    if (!token.value.empty()) {
                        tag += "=" + token.value;
                    }
                    tag += "}";
                    open_tags = tag + open_tags;
                }
            }
        }
    }
}

void Canvas::render_text(const std::string& text, float x, float y) {
    text_renderer->render_formatted(font, formatter, *style, std::round(x), std::round(y), text);
}

void Canvas::set_font(const std::string& font_file) {
    if (font_file == get_font_filename())
        return;
    if (!file_exists(font_file))
        throw std::runtime_error("Couldn't read font file " + font_file);
    font = xd::create<xd::font>(font_file);
    redraw_needed = true;
}

void Canvas::link_font(const std::string& type, const std::string& font_file) {
    if (file_exists(font_file)) {
        font->link_font(type, xd::create<xd::font>(font_file));
    } else {
        LOGGER_W << "Couldn't read '" << type << "' font file " << font_file;
    }
}

bool Canvas::should_redraw(int time) const {
    bool redraw_children = std::any_of(std::begin(children), std::end(children),
        [time](const std::unique_ptr<Canvas>& c) { return c->should_redraw(time);  });
    static int ms_between_refresh = 1000 / Configurations::get<int>("debug.canvas-fps");
    bool time_to_update = time - last_drawn_time > ms_between_refresh;
    // Sprites and images are always redrawn, might change it later
    return redraw_needed || type != Canvas::Type::TEXT
        || redraw_children || time_to_update;
}

void Canvas::mark_as_drawn(int time) {
    redraw_needed = false;
    last_drawn_time = time;
}
