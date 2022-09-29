#include "../../include/canvas/base_canvas.hpp"
#include "../../include/utility/color.hpp"
#include "../../include/game.hpp"
#include "../../include/configurations.hpp"

Base_Canvas::Base_Canvas(Game& game, Base_Canvas::Type type, xd::vec2 position) :
    game(game),
    priority(0),
    type(type),
    children_type(type),
    position(position),
    color(1.0f),
    visible(false),
    camera_relative(true),
    redraw_needed(true),
    last_drawn_time(0),
    background_visible(false),
    background_color{ hex_to_color(Configurations::get<std::string>("text.background-color")) },
    paused_game_canvas(game.is_paused()),
    last_camera_position(0.0f, 0.0f) {}

void Base_Canvas::setup_fbo() {
    if (!Configurations::get<bool>("debug.use-fbo") || !xd::framebuffer::extension_supported()) {
        return;
    }

    auto width = static_cast<int>(game.game_width());
    auto height = static_cast<int>(game.game_height());

    fbo_texture = std::make_shared<xd::texture>(width, height, nullptr,
        xd::vec4(0), GL_CLAMP, GL_CLAMP, GL_NEAREST, GL_NEAREST);
}

void Base_Canvas::remove_child(const std::string& child_name) {
    children.erase(
        std::remove_if(children.begin(), children.end(),
            [&child_name](std::unique_ptr<Base_Canvas>& c) { return c->get_name() == child_name; }
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

void Base_Canvas::inherit_properties(const Base_Canvas& parent) {
    // Child inherits parent properties
    set_color(parent.get_color());
    set_scissor_box(parent.get_scissor_box());
}

bool Base_Canvas::should_update() const {
    if (!visible) return false;
    return game.is_paused() == paused_game_canvas;
}

bool Base_Canvas::should_redraw(int time) const {
    const bool redraw_children = std::any_of(std::begin(children), std::end(children),
        [time](const std::unique_ptr<Base_Canvas>& c) { return c->should_redraw(time);  });
    static int ms_between_refresh = 1000 / Configurations::get<int>("debug.canvas-fps");
    const bool time_to_update = time - last_drawn_time > ms_between_refresh;
    return redraw_needed || redraw_children || time_to_update;
}

void Base_Canvas::mark_as_drawn(int time) {
    redraw_needed = false;
    last_drawn_time = time;
}
