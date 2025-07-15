#include "canvas_updater.hpp"
#include "base_canvas.hpp"
#include "../map/map.hpp"

void Canvas_Updater::update(Map& map) {
    auto& canvases = map.get_canvases();
    for (auto& weak_canvas : canvases) {
        auto canvas = weak_canvas.ptr.lock();
        if (canvas && canvas->should_update()) {
            canvas->update();
        }
    }
}
