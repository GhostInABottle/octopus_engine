#include "../../include/camera.hpp"
#include "../../include/canvas/base_canvas.hpp"
#include "../../include/canvas/base_image_canvas.hpp"
#include "../../include/canvas/canvas_renderer.hpp"
#include "../../include/configurations.hpp"
#include "../../include/game.hpp"
#include "../../include/map/map.hpp"
#include "../../include/utility/math.hpp"

Canvas_Renderer::Canvas_Renderer(Game& game, Camera& camera)
    : game(game)
    , camera(camera)
    , fbo_supported(Configurations::get<bool>("graphics.use-fbo", "debug.use-fbo")
        && xd::framebuffer::extension_supported())
    , background_margins(
        Configurations::get<int>("text.background-margin-left"),
        Configurations::get<int>("text.background-margin-top"),
        Configurations::get<int>("text.background-margin-right"),
        Configurations::get<int>("text.background-margin-bottom"))
{}

void Canvas_Renderer::render(Map& map) {
    camera.draw_map_tint();

    auto& canvases = map.get_canvases();
    for (auto& weak_canvas : canvases) {
        auto canvas = weak_canvas.ptr.lock();
        if (!canvas || !canvas->is_visible())
            continue;

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

void Canvas_Renderer::setup_framebuffer(const Base_Canvas& canvas) {
    auto scissor_box = canvas.get_scissor_box();
    auto has_scissor_box = scissor_box.w > 0;
    if (has_scissor_box) {
        // Disable scissor testing to make sure that glClear wipes the entire texture
        // This prevents visual glitches caused by reused textures on mac/linux
        camera.disable_scissor_test();
    }

    auto& framebuffer = game.get_framebuffer();
    framebuffer.attach_color_texture(*canvas.get_fbo_texture(), 0);
    auto [complete, error] = framebuffer.check_complete();
    if (!complete) throw std::runtime_error(error);

    auto game_width = game.game_width();
    auto game_height = game.game_height();

    camera.set_viewport(xd::rect{0, 0, game_width, game_height});

    // Clear the screen then restore the old color
    auto old_color = camera.get_clear_color();
    camera.set_clear_color(xd::vec4{0.0f});
    camera.clear_color_buffer();
    camera.set_clear_color(old_color);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (!has_scissor_box) return;

    // Use a texture-sized viewport when calculating scissor box
    xd::rect viewport{0, 0,
        static_cast<float>(game_width),
        static_cast<float>(game_height)};
    camera.enable_scissor_test(scissor_box, viewport);
}

void Canvas_Renderer::render_framebuffer(const Base_Canvas& canvas, const Base_Canvas& root) {
    game.get_framebuffer().unbind();
    if (canvas.get_scissor_box().w > 0) {
        // Scissor test was already applied when drawing to FBO
        camera.disable_scissor_test();
    }

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    float x = 0.0f, y = 0.0f;
    if (!canvas.is_camera_relative()) {
        // Since the camera might have moved since last draw, we need to update
        // the position of non-camera-relative canvases
        auto camera_pos = camera.get_pixel_position();
        auto last_pos = canvas.get_last_camera_position();
        x = last_pos.x - camera_pos.x;
        y = camera_pos.y - last_pos.y;
    }
    batch.add(canvas.get_fbo_texture(), x, y, xd::vec4(1.0f));

    camera.use_calculated_viewport();

    xd::transform_geometry geometry;
    geometry.projection().load(
        xd::ortho<float>(
            0, static_cast<float>(game.game_width()), // left, right
            0, static_cast<float>(game.game_height()), // bottom, top
            -1, 1) // near, far
    );
    geometry.model_view().identity();

    draw(geometry.mvp(), root);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Canvas_Renderer::render_canvas(Base_Canvas& canvas, Base_Canvas* parent, Base_Canvas* root) {
    if (!canvas.is_visible() || check_close(canvas.get_opacity(), 0.0f)) {
        if (!parent) {
            canvas.mark_as_drawn(game.window_ticks());
        }
        return;
    }

    render_background(canvas, parent);

    // Drawing to a framebuffer that updates less frequently improves performance,
    // especially for text. For sprites/images the improvement isn't as significant.
    // and they are more likely to be moved/animated. Therefore we restrict this
    // optimization for images/sprites in a child/parent hierarchy
    auto& root_parent = root ? *root : canvas;
    bool has_children = canvas.get_child_count() > 0;
    bool is_text = canvas.get_type() == Base_Canvas::Type::TEXT;
    bool using_fbo = !parent && fbo_supported && (is_text || has_children);
    bool individual = !parent && !has_children;
    bool redraw = should_redraw(canvas)
        || (parent && should_redraw(*parent))
        || (!is_text && (individual || !fbo_supported));

    if (redraw) {
        if (using_fbo) {
            setup_framebuffer(canvas);
            canvas.set_last_camera_position(camera.get_pixel_position());
        }

        if (is_text && !batch.empty()) {
            // Text rendering isn't batched, so draw any batched images
            // and clear the batch before drawing any text
            draw(camera.get_mvp(), root_parent);
        }

        // Render canvas and children
        canvas.render(camera, batch, parent);

        for (size_t i = 0; i < canvas.get_child_count(); ++i) {
            auto& child = *canvas.get_child_by_index(i);
            render_canvas(child, &canvas, &root_parent);
        }

        // Mark children and parent as drawn. Children are marked separately
        // to avoid changing parent's should_draw while iterating children
        for (size_t i = 0; i < canvas.get_child_count(); ++i) {
            canvas.get_child_by_index(i)->mark_as_drawn(game.window_ticks());
        }

        if (!parent) {
            canvas.mark_as_drawn(game.window_ticks());
            if (!batch.empty()) {
                draw(camera.get_mvp(), root_parent);
            }
        }
    }

    if (using_fbo) {
        render_framebuffer(canvas, root_parent);
    }
}

void Canvas_Renderer::render_background(Base_Canvas& canvas, Base_Canvas* parent) {
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
        auto camera_pos = camera.get_pixel_position();
        rect.x += camera_pos.x;
        rect.y += camera_pos.y;
    }

    auto color = canvas.get_background_color();
    color.a *= canvas.get_opacity();
    if (check_close(color.a, 0.0f)) return;
    camera.draw_rect(rect, color);
}
bool Canvas_Renderer::should_redraw(const Base_Canvas& canvas) {
    return canvas.should_redraw(game.window_ticks()) || !fbo_supported;
}

void Canvas_Renderer::draw(const xd::mat4 mvp, const Base_Canvas& root) {
    xd::shader_uniforms uniforms{mvp};

    auto root_image = dynamic_cast<const Base_Image_Canvas*>(&root);
    auto outline_color = root_image ? root_image->get_outline_color() : std::nullopt;
    if (outline_color.has_value()) {
        batch.set_outline_color(outline_color.value());
        batch.draw_outlined(uniforms);
    } else {
        batch.draw(uniforms);
    }

    batch.clear();
}
