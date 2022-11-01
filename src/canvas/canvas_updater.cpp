#include "../../include/canvas/canvas_updater.hpp"
#include "../../include/canvas/base_canvas.hpp"
#include "../../include/map.hpp"

void Canvas_Updater::update(Map& map) {
    map.erase_canvases(
        [](const Map::Canvas_Ref& weak_canvas) {
            auto canvas = weak_canvas.ptr.lock();
            if (canvas && canvas->should_update()) {
                canvas->update();
            }
            return !canvas;
        }
    );
}
