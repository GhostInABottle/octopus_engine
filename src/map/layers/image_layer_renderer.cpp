#include "../../../include/camera.hpp"
#include "../../../include/game.hpp"
#include "../../../include/map/layers/image_layer.hpp"
#include "../../../include/map/layers/image_layer_renderer.hpp"
#include "../../../include/sprite.hpp"

void Image_Layer_Renderer::render(Map& map) {
    if (!layer.is_visible()) return;

    batch.clear();
    auto& image_layer = static_cast<const Image_Layer&>(layer);
    xd::vec2 pos;
    if (image_layer.is_fixed()) {
        pos = camera.get_pixel_position();
    }

    auto sprite = image_layer.get_sprite();
    if (sprite) {
        sprite->render(batch, image_layer, pos);
    } else {
        auto texture = image_layer.get_texture();
        xd::vec4 color = image_layer.get_color();
        color.a *= layer.get_opacity();

        if (image_layer.is_repeating()) {
            xd::vec2 tex_size = xd::vec2(texture->width(), texture->height());
            xd::rect src(-image_layer.get_position(), tex_size);
            batch.add(texture, src, pos.x, pos.y, 0.0f, 1.0f, color);
        } else {
            batch.add(texture, pos.x, pos.y, color);
        }
    }

    batch.draw(xd::shader_uniforms{camera.get_mvp(), map.get_game().ticks()});
}
