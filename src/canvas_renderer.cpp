#include "../include/canvas_renderer.hpp"
#include "../include/canvas.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/camera.hpp"
#include "../include/sprite_data.hpp"
#include "../include/configurations.hpp"
#include "../include/utility/math.hpp"

Canvas_Renderer::Canvas_Renderer(Game& game, Camera& camera)
    : game(game),
    camera(camera),
    fbo_supported(Configurations::get<bool>("debug.use-fbo") && xd::framebuffer::extension_supported()),
    background_margins(
        Configurations::get<float>("text.background-margin-left"),
        Configurations::get<float>("text.background-margin-top"),
        Configurations::get<float>("text.background-margin-right"),
        Configurations::get<float>("text.background-margin-bottom"))
{}

void Canvas_Renderer::render(Map& map) {
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
                camera.enable_scissor_test(scissor);
            }

            render_canvas(*canvas);

            if (has_scissor_box) {
                camera.disable_scissor_test();
            }
        }
    }
}

void Canvas_Renderer::setup_framebuffer(const Canvas& canvas) {
    auto scissor_box = canvas.get_scissor_box();
    if (scissor_box.w > 0) {
        // Use a texture-sized viewport when calculating scissor box
        xd::rect viewport = xd::rect(0, 0, game.game_width(), game.game_height());
        camera.enable_scissor_test(scissor_box, viewport);
    }

    auto framebuffer = canvas.get_framebuffer();
    framebuffer->attach_color_texture(*canvas.get_fbo_texture(), 0);
    framebuffer->bind();
    glViewport(0, 0, static_cast<int>(game.game_width()), static_cast<int>(game.game_height()));
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    camera.set_clear_color();
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    auto [complete, error] = framebuffer->check_complete();
    if (!complete) throw std::runtime_error(error);
}

void Canvas_Renderer::render_framebuffer(const Canvas& canvas, const Canvas& root) {
    canvas.get_framebuffer()->unbind();
    if (canvas.get_scissor_box().w > 0) {
        // Scissor test was already applied when drawing to FBO
        camera.disable_scissor_test();
    }
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    batch.add(canvas.get_fbo_texture(), 0, 0, xd::vec4(1.0f));
    camera.update_viewport();
    xd::transform_geometry geometry;
    geometry.projection().push(
        xd::ortho<float>(
            0, static_cast<float>(game.game_width()), // left, right
            0, static_cast<float>(game.game_height()), // bottom, top
            -1, 1 // near, far
            )
    );
    geometry.model_view().push(xd::mat4());
    draw(geometry.mvp(), root);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Canvas_Renderer::render_canvas(Canvas& canvas, Canvas* parent, Canvas* root) {
    if (!canvas.is_visible() || check_close(canvas.get_opacity(), 0.0f)) {
        canvas.mark_as_drawn(game.ticks());
        return;
    }
    // Draw text background
    render_background(canvas, parent);

    // Drawing to a framebuffer that updates less frequently improves performance,
    // especially for text. For sprites/images the improvement isn't as significant.
    // and they are more likely to be moved/animated. Therefore we ristrict this
    // optimization for images/sprites in a child/parent hierarchy
    auto& root_parent = root ? *root : canvas;
    bool has_children = canvas.get_child_count() > 0;
    bool is_text = canvas.get_type() == Canvas::Type::TEXT;
    bool using_fbo = !parent && fbo_supported && (is_text || has_children);
    bool individual = !(parent || has_children);
    bool redraw = should_redraw(canvas)
        || parent && should_redraw(*parent)
        || !is_text && (individual || !fbo_supported);

    if (redraw) {
        // Setup framebuffer for top parent canvas
        if (using_fbo) {
            setup_framebuffer(canvas);
        }
        // Render canvas and children
        if (canvas.get_type() == Canvas::Type::TEXT) {
            render_text(canvas, parent);
        } else {
            render_image(canvas, parent);
        }
        for (size_t i = 0; i < canvas.get_child_count(); ++i) {
            auto& child = *canvas.get_child(i);
            render_canvas(child, &canvas, &root_parent);
        }
        // Mark children and parent as drawn. Children are marked separately
        // to avoid changing parent's should_draw while iterating children
        for (size_t i = 0; i < canvas.get_child_count(); ++i) {
            canvas.get_child(i)->mark_as_drawn(game.ticks());
        }
        if (!parent) {
            canvas.mark_as_drawn(game.ticks());
            if (!batch.empty()) {
                draw(camera.get_mvp(), root_parent);
            }
        }
    }
    if (using_fbo) {
        render_framebuffer(canvas, root_parent);
    }
}

void Canvas_Renderer::render_text(Canvas& canvas, Canvas* parent) {
    auto style = canvas.get_style();
    style->color().a = canvas.get_opacity();
    auto& lines = canvas.get_text_lines();
    xd::vec2 pos = canvas.get_position();
    auto camera_pos = camera.get_position();
    if (parent) {
        pos += parent->get_position();
    }

    for (auto line : lines) {
        float draw_x = pos.x;
        float draw_y = pos.y;
        if (!canvas.is_camera_relative()) {
            draw_x -= camera_pos.x;
            draw_y -= camera_pos.y;
        }

        canvas.render_text(line, draw_x, draw_y);
        pos.y += style->line_height();
    }
}

void Canvas_Renderer::render_image(Canvas& canvas, Canvas* parent) {
    xd::vec2 pos = canvas.get_position();
    auto camera_pos = camera.get_position();
    pos += camera_pos;
    xd::vec4 color = canvas.get_color();
    if (parent) {
        pos += parent->get_position();
    }

    xd::vec2 mag = canvas.get_magnification();
    if (auto sprite = canvas.get_sprite()) {
        sprite->render(batch, xd::vec2(pos.x, pos.y), 1.0f, mag, color);
    } else {
        float angle = canvas.get_angle();
        batch.add(canvas.get_image_texture(), pos.x, pos.y,
            xd::radians(angle), mag, color, canvas.get_origin());
    }
}

void Canvas_Renderer::render_background(Canvas& canvas, Canvas* parent) {
    if (!canvas.has_background()) return;
    auto rect = canvas.get_background_rect();
    rect.x -= background_margins.x;
    rect.y -= background_margins.y;
    rect.w += background_margins.w;
    rect.h += background_margins.h;

    if (parent) {
        auto parent_pos = parent->get_position();
        rect.x += parent_pos.x;
        rect.y += parent_pos.y;
    }

    if (canvas.is_camera_relative()) {
        auto camera_pos = camera.get_position();
        rect.x += camera_pos.x;
        rect.y += camera_pos.y;
    }

    auto color = canvas.get_background_color();
    color.a *= canvas.get_opacity();
    if (check_close(color.a, 0.0f)) return;
    camera.draw_rect(rect, color);
}
bool Canvas_Renderer::should_redraw(const Canvas& canvas) {
    return canvas.should_redraw(game.ticks()) || !fbo_supported;
}

void Canvas_Renderer::draw(const xd::mat4 mvp, const Canvas& root) {
    xd::shader_uniforms uniforms{mvp};
    if (root.has_image_outline()) {
        batch.set_outline_color(root.get_image_outline_color());
        batch.draw_outlined(uniforms);
    } else {
        batch.draw(uniforms);
    }
    batch.clear();
}