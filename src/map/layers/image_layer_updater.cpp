
#include "image_layer.hpp"
#include "image_layer_updater.hpp"
#include "../map.hpp"
#include "../../game.hpp"

void Image_Layer_Updater::update(Map& map) {
    if (!layer.is_visible()) return;

    auto& image_layer = static_cast<Image_Layer&>(layer);
    auto sprite = image_layer.get_sprite();
    if (sprite) {
        sprite->update();
    }

    if (!image_layer.is_repeating()) return;

    auto position = image_layer.get_position();
    auto new_position = position + image_layer.get_velocity();
    image_layer.set_position(new_position);

    auto& game = map.get_game();
    auto game_width = static_cast<float>(game.game_width());
    auto game_height = static_cast<float>(game.game_height());
    auto reset_pos = new_position.x > game_width &&
        new_position.y > game_height;
    if (reset_pos) {
        image_layer.set_position(xd::vec2(0.0f, 0.0f));
    }
}
