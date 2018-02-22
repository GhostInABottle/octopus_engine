#include "../include/canvas_renderer.hpp"
#include "../include/canvas.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/camera.hpp"
#include "../include/sprite_data.hpp"
#include "../include/configurations.hpp"
#include "../include/utility.hpp"

Canvas_Renderer::Canvas_Renderer() {
    fbo_supported = Configurations::get<bool>("debug.use-fbo")
        && xd::framebuffer::extension_supported();
}

void Canvas_Renderer::render(Map& map) {
    auto& game = map.get_game();
    auto& canvases = map.get_canvases();
    for (auto& weak_canvas : canvases) {
        auto canvas = weak_canvas.lock();
        if (!canvas)
            continue;
        if (canvas->is_visible()) {
            // Limit drawing to scissor rectangle, if specified
            xd::rect scissor = canvas->get_scissor_box();
            bool has_scissor_box = scissor.w > 0;
            if (has_scissor_box) {
                game.get_camera()->enable_scissor_test(scissor);
            }
            batch.clear();

            render_canvas(game, canvas.get());

            if (drawn_to_batch) {
                batch.draw(game.get_mvp());
            }

            glEnable(GL_BLEND);
            if (has_scissor_box) {
                game.get_camera()->disable_scissor_test();
            }
        }
    }
}

void Canvas_Renderer::setup_framebuffer(Game& game, Canvas* canvas) {
    auto scissor_box = canvas->get_scissor_box();
    if (scissor_box.w > 0) {
        // Use a texture-sized viewport when calculating scissor box
        xd::rect viewport = xd::rect(0, 0, game.game_width, game.game_height);
        game.get_camera()->enable_scissor_test(scissor_box, viewport);
    }
    auto framebuffer = canvas->get_framebuffer();
    framebuffer->attach_color_texture(canvas->get_fbo_texture(), 0);
    framebuffer->bind();
    glViewport(0, 0, game.game_width, game.game_height);
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPopAttrib();
    auto fb_complete = framebuffer->check_complete();
    if (!std::get<0>(fb_complete))
        throw std::exception(std::get<1>(fb_complete).c_str());
}

void Canvas_Renderer::render_framebuffer(Game& game, Canvas* canvas) {
    canvas->get_framebuffer()->unbind();
    if (canvas->get_scissor_box().w > 0) {
        // Scissor test was already applied when drawing to FBO
        game.get_camera()->disable_scissor_test();
    }
    glDisable(GL_BLEND);
    batch.add(canvas->get_fbo_texture(), 0, 0, xd::vec4(1.0f));
    game.get_camera()->update_viewport();
    xd::transform_geometry geometry;
    geometry.projection().push(
        xd::ortho<float>(
            0, static_cast<float>(game.game_width), // left, right
            0, static_cast<float>(game.game_height), // bottom, top
            -1, 1 // near, far
            )
    );
    geometry.model_view().push(xd::mat4());
    batch.draw(geometry.mvp());
    batch.clear();
    drawn_to_batch = false;
}

void Canvas_Renderer::render_canvas(Game& game, Canvas* canvas, Canvas* parent) {
    if (!canvas->is_visible()) {
        canvas->mark_as_drawn(game.ticks());
        return;
    }
    // Text canvases with text children are drawn to one
    // framebuffer object as an optimization
    auto children_type = canvas->get_children_type();
    auto child_count = canvas->get_child_count();
    bool using_fbo = false;
    bool redraw = should_redraw(game, canvas);
    bool is_text_child = parent && parent->get_children_type() == Canvas::Type::TEXT;
    bool redraw_parent = is_text_child && should_redraw(game, parent);
    if (canvas->get_type() == Canvas::Type::TEXT) {
        // Setup framebuffer for top parent canvas
        if (!parent && fbo_supported) {
            if (redraw) {
                setup_framebuffer(game, canvas);
            }
            using_fbo = true;
        }

        if (redraw || redraw_parent) {
            render_text(game, canvas, parent);
        }

        bool has_text_children = child_count > 0 && children_type == Canvas::Type::TEXT;
        if (fbo_supported && !is_text_child && !has_text_children) {
            // Finish up previous FBO
            render_framebuffer(game, canvas);
            using_fbo = false;
        }
    } else {
        render_image(game, canvas, parent);
        // If image canvas has text children, render them in one FBO
        if (fbo_supported && child_count > 0 && children_type == Canvas::Type::TEXT) {
            batch.draw(game.get_mvp());
            if (should_redraw(game, canvas->get_child(0))) {
                setup_framebuffer(game, canvas);
            }
            using_fbo = true;
        }
    }
    // Render children
    for (size_t i = 0; i < canvas->get_child_count(); ++i) {
        auto child = canvas->get_child(i);
        // Individual text children in mixed group are drawn to their own FBO
        if (fbo_supported && child->get_type() == Canvas::Type::TEXT && !using_fbo) {
            if (drawn_to_batch) {
                // Finish up any previous image drawing
                batch.draw(game.get_mvp());
                drawn_to_batch = false;
            }
            if (should_redraw(game, child)) {
                setup_framebuffer(game, child);
            }
        }
        render_canvas(game, child, canvas);
    }
    // Need to mark children separately to avoid changing parent's
    // should_draw while iterating children
    for (size_t i = 0; i < canvas->get_child_count(); ++i) {
        canvas->get_child(i)->mark_as_drawn(game.ticks());
    }
    if (using_fbo) {
        render_framebuffer(game, canvas);
    }
    if (redraw && !redraw_parent)
        canvas->mark_as_drawn(game.ticks());
}

void Canvas_Renderer::render_text(Game& game, Canvas* canvas, Canvas* parent) {
    auto style = canvas->get_style();
    style->color().a = canvas->get_opacity();
    auto& lines = canvas->get_text_lines();
    xd::vec2 pos = canvas->get_position();
    auto camera_pos = game.get_camera()->get_position();
    if (parent) {
        pos += parent->get_position();
        // Override setting with parent's, where it converges from default
        if (parent->get_type() == Canvas::Type::TEXT) {
            if (check_close(style->color(), xd::vec4(1.0f)))
                style->color() = parent->get_color();
            static int default_font_size = Configurations::get<int>("font.size");
            if (canvas->get_font_size() == default_font_size)
                style->size() = parent->get_font_size();
            if (!canvas->has_text_type() && parent->has_text_type())
                style->type() = parent->get_text_type();
            if (!canvas->has_outline() && parent->has_outline())
                style->outline(parent->get_text_outline_width(),
                    parent->get_text_outline_color());
            if (!canvas->has_shadow() && parent->has_shadow()) {
                auto offset = parent->get_text_shadow_offset();
                style->shadow(offset.x, offset.y, parent->get_text_shadow_color());
            }
        }
    }
    for (auto line : lines) {
        float draw_x = pos.x;
        float draw_y = pos.y;
        if (!canvas->is_camera_relative()) {
            draw_x -= camera_pos.x;
            draw_y -= camera_pos.y;
        }

        canvas->render_text(line, draw_x, game.game_height - draw_y);
        pos.y += style->line_height();
    }
}

void Canvas_Renderer::render_image(Game& game, Canvas* canvas, Canvas* parent) {
    xd::vec2 pos = canvas->get_position();
    auto camera_pos = game.get_camera()->get_position();
    pos += camera_pos;
    xd::vec4 color = canvas->get_color();
    if (parent) {
        pos += parent->get_position();
        if (check_close(color, xd::vec4(1.0f)))
            color = parent->get_color();
    }
    if (auto sprite = canvas->get_sprite()) {
        sprite->render(batch, xd::vec2(pos.x, pos.y), 1.0f, color);
    } else {
        float angle = canvas->get_angle();
        xd::vec2 magnification = canvas->get_magnification();
        if (parent) {
            if (check_close(angle, 0.0f))
                angle = parent->get_angle();
            if (check_close(magnification, xd::vec2(1.0f)))
                magnification = parent->get_magnification();
        }
        batch.add(canvas->get_image_texture(), pos.x, pos.y,
            xd::radians(angle), magnification, color, canvas->get_origin());
    }
    drawn_to_batch = true;
}

bool Canvas_Renderer::should_redraw(Game& game, Canvas* canvas) {
    return canvas->should_redraw(game.ticks()) || !fbo_supported;
}