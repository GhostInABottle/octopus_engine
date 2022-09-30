#include "../../include/canvas/canvas_updater.hpp"
#include "../../include/canvas/base_canvas.hpp"
#include "../../include/map.hpp"

void Canvas_Updater::update(Map& map) {
    map.erase_canvases(
        [](const std::weak_ptr<Base_Canvas>& weak_canvas) {
            auto canvas = weak_canvas.lock();
            if (canvas) {
                if (canvas->should_update()) {
                    canvas->update();
                }
            }
            return !canvas;
        }
    );
}
