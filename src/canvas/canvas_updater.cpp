#include "../../include/canvas/canvas_updater.hpp"
#include "../../include/canvas/canvas.hpp"
#include "../../include/map.hpp"

void Canvas_Updater::update(Map& map) {
    map.erase_canvases(
        [](const std::weak_ptr<Canvas>& weak_canvas) {
            auto canvas = weak_canvas.lock();
            if (canvas) {
                if (canvas->should_update() && canvas->get_sprite()) {
                    canvas->get_sprite()->update();
                }
            }
            return !canvas;
        }
    );
}
