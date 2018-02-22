#include "../include/canvas_updater.hpp"
#include "../include/map.hpp"
#include "../include/canvas.hpp"
#include <algorithm>

void Canvas_Updater::update(Map& map) {
    auto& canvases = map.get_canvases();
    canvases.erase(
        std::remove_if(
            std::begin(canvases), std::end(canvases),
            [](const std::weak_ptr<Canvas>& weak_canvas) {
                auto canvas = weak_canvas.lock();
                if (canvas) {
                    if (canvas->is_visible() && canvas->get_sprite()) {
                        canvas->get_sprite()->update();
                    }
                }
                return !canvas;
            }
        ), std::end(canvases)
    );
}
