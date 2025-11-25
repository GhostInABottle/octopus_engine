#include "tile_layer_renderer.hpp"
#include "tile_layer.hpp"
#include "../map.hpp"
#include "../../camera.hpp"
#include "../../game.hpp"

void Tile_Layer_Renderer::render(Map& map) {
    if (!layer.is_visible()) return;

    if (needs_redraw) {
        batch.clear();
        auto& tiles = static_cast<const Tile_Layer&>(layer).get_tiles();
        auto layer_opacity = layer.get_opacity();

        for (int y = 0; y < map.get_height(); ++y) {
            for (int x = 0; x < map.get_width(); ++x) {
                const int i = y * map.get_width() + x;
                const Tileset& tileset = map.get_tileset(0);
                const int tile_id = tiles[i];
                if (tile_id > 0) {
                    const int tile_index = tile_id - tileset.first_id;
                    const xd::rect src = tileset.tile_source_rect(tile_index);
                    const float tile_x = static_cast<float>(x * map.get_tile_width());
                    const float tile_y = static_cast<float>(y * map.get_tile_height());
                    const xd::vec4 color(1.0f, 1.0f, 1.0f, layer_opacity);
                    batch.add(tileset.image_texture, src, tile_x, tile_y, color);
                }
            }
        }
        cache = batch.create_batches();
        needs_redraw = false;
    }

    batch.draw(camera.get_mvp(), cache);
}
