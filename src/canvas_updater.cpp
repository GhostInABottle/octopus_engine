#include "../include/canvas_updater.hpp"
#include "../include/map.hpp"
#include "../include/canvas.hpp"

void Canvas_Updater::update(Map& map) {
    auto& canvases = map.get_canvases();
    for (auto& canvas : canvases) {
        if (canvas->is_visible()) {
            if (auto sprite = canvas->get_sprite())
                sprite->update();
        }
    }
}
