#include "../../include/game.hpp"
#include "../../include/layers/image_layer.hpp"
#include "../../include/layers/image_layer_updater.hpp"
#include "../../include/map.hpp"

void Image_Layer_Updater::update(Map& map) {
    if (!layer.visible) return;

    auto& image_layer = static_cast<Image_Layer&>(layer);
    if (image_layer.sprite) {
        image_layer.sprite->update();
    }

    if (!image_layer.repeat) return;

    image_layer.position += image_layer.velocity;

    auto& game = map.get_game();
    auto game_width = static_cast<float>(game.game_width());
    auto game_height = static_cast<float>(game.game_height());
    auto reset_pos = image_layer.position.x > game_width &&
        image_layer.position.y > game_height;
    if (reset_pos) {
        image_layer.position = xd::vec2(0.0f, 0.0f);
    }
}
