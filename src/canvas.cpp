#include <cmath>
#include <stdexcept>
#include "../include/xd/graphics/framebuffer.hpp"
#include "../include/canvas.hpp"
#include "../include/utility/color.hpp"
#include "../include/utility/direction.hpp"
#include "../include/utility/file.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/sprite_data.hpp"
#include "../include/xd/asset_manager.hpp"
#include "../include/configurations.hpp"

Canvas::Canvas(Game& game, xd::vec2 position) :
    game(game),
    priority(0),
    type(Canvas::Type::IMAGE),
    children_type(Canvas::Type::IMAGE),
    position(position),
    magnification(1.0f, 1.0f),
    color(1.0f),
    visible(false),
    camera_relative(false),
    redraw_needed(true),
    last_drawn_time(0),
    permissive_tag_parsing(false),
    use_outline_shader(false),
    outline_color(hex_to_color(Configurations::get<std::string>("game.object-outline-color"))),
    background_visible(false),
    background_color{hex_to_color(Configurations::get<std::string>("text.background-color"))},
    paused_game_canvas(game.is_paused()),
    last_camera_position(0.0f, 0.0f) {}

Canvas::Canvas(Game& game, const std::string& sprite, const std::string& pose_name, xd::vec2 position) : Canvas(game, position) {
    camera_relative = false;
    children_type = Canvas::Type::SPRITE;
    set_sprite(game, sprite, pose_name);
}

Canvas::Canvas(Game& game, const std::string& filename, xd::vec2 position, xd::vec4 trans) : Canvas(game, position) {
    camera_relative = false;
    children_type = Canvas::Type::IMAGE;
    set_image(filename, trans);
}

Canvas::Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative, bool is_child) : Canvas(game, position) {
    font = game.get_font();
    style = std::make_unique<xd::font_style>(game.get_font_style());
    this->camera_relative = camera_relative;
    if (!is_child) {
        setup_fbo();
    }
    type = Canvas::Type::TEXT;
    children_type = type;
    set_text(text);
}

void Canvas::setup_fbo() {
    if (!Configurations::get<bool>("debug.use-fbo") || !xd::framebuffer::extension_supported()) {
        return;
    }

    auto width = static_cast<int>(game.game_width());
    auto height = static_cast<int>(game.game_height());

    fbo_texture = std::make_shared<xd::texture>(width, height, nullptr,
        xd::vec4(0), GL_CLAMP, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    framebuffer = std::make_shared<xd::framebuffer>();
}

void Canvas::set_image(std::string image_filename, xd::vec4 trans) {
    file_utilities::normalize_slashes(image_filename);
    if (filename == image_filename)
        return;
    type = Canvas::Type::IMAGE;
    filename = image_filename;
    image_texture = std::make_shared<xd::texture>(image_filename,
        trans, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    redraw_needed = true;
}

void Canvas::set_sprite(Game& game, const std::string& sprite_filename, const std::string& pose_name) {
    if (filename == sprite_filename)
        return;
    type = Canvas::Type::SPRITE;
    filename = sprite_filename;
    sprite = std::make_unique<Sprite>(game,
        Sprite_Data::load(game.get_asset_manager(), sprite_filename, game.get_audio()));
    set_pose(pose_name, "", Direction::NONE);
    redraw_needed = true;
}

std::string Canvas::get_pose_name() {
    return get_sprite() ? get_sprite()->get_pose().name : "";
}

std::string Canvas::get_pose_state() {
    return get_sprite() ? get_sprite()->get_pose().state : "";
}

Direction Canvas::get_pose_direction()
{
    return get_sprite() ? get_sprite()->get_pose().direction : Direction::NONE;
}

void Canvas::remove_child(const std::string& child_name) {
    children.erase(
        std::remove_if(children.begin(), children.end(),
            [&child_name](std::unique_ptr<Canvas>& c) { return c->get_name() == child_name; }
        ), children.end());
    // Update children type if needed
    if (!children.empty() && children_type == Type::MIXED) {
        auto child_type = children.front()->get_type();
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
        if (parent.has_text_outline()) {
            set_text_outline_color(parent.get_text_outline_color());
            set_text_outline_width(parent.get_text_outline_width());
        }
        if (parent.has_shadow()) {
            set_text_shadow_color(parent.get_text_shadow_color());
            set_text_shadow_offset(parent.get_text_shadow_offset());
        }
    }
}

void Canvas::set_text(const std::string& new_text) {
    if (text == new_text && !text.empty())
        return;
    text = new_text;
    text_lines = Text_Parser::split_to_lines(new_text, permissive_tag_parsing);
    redraw_needed = true;
}

void Canvas::render_text(const std::string& text_to_render, float x, float y) const {
    game.render_text(*font, *style, x, y, text_to_render);
}

void Canvas::set_font(const std::string& font_file) {
    if (font_file == get_font_filename())
        return;
    font = game.create_font(font_file);
    redraw_needed = true;
}

void Canvas::link_font(const std::string& font_type, const std::string& font_file) {
    if (!file_utilities::file_exists(font_file)) {
        throw std::runtime_error("Couldn't read font file " + font_file);
    }

    font->link_font(font_type, game.create_font(font_file));
}

bool Canvas::should_update() const {
    if (!visible) return false;
    return game.is_paused() == paused_game_canvas;
}

float Canvas::get_text_width(const std::string& text) {
    return game.text_width(text, font.get(), style.get());
}

bool Canvas::should_redraw(int time) const {
    const bool redraw_children = std::any_of(std::begin(children), std::end(children),
        [time](const std::unique_ptr<Canvas>& c) { return c->should_redraw(time);  });
    static int ms_between_refresh = 1000 / Configurations::get<int>("debug.canvas-fps");
    const bool time_to_update = time - last_drawn_time > ms_between_refresh;
    return redraw_needed || redraw_children || time_to_update;
}

void Canvas::mark_as_drawn(int time) {
    redraw_needed = false;
    last_drawn_time = time;
}
