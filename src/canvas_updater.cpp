#include "../include/canvas_updater.hpp"
#include "../include/map.hpp"
#include "../include/canvas.hpp"

void Canvas_Updater::update(Map& map) {
    map.erase_canvases(
        [](const std::weak_ptr<Canvas>& weak_canvas) {
            auto canvas = weak_canvas.lock();
            if (canvas) {
                if (canvas->is_visible() && canvas->get_sprite()) {
                    canvas->get_sprite()->update();
                }
            }
            return !canvas;
        }
    );
}
