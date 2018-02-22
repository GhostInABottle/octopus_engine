#include "../include/image_layer_updater.hpp"
#include "../include/image_layer.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"

void Image_Layer_Updater::update(Map& map) {
    if (!layer->visible)
        return;
    auto image_layer = static_cast<Image_Layer*>(layer);
    if (image_layer->sprite)
        image_layer->sprite->update();
    if (image_layer->repeat) {
        image_layer->position += image_layer->velocity;
        if (image_layer->position.x > map.get_game().game_width &&
                image_layer->position.y > map.get_game().game_height)
            image_layer->position = xd::vec2(0.0f, 0.0f);
    }
}
