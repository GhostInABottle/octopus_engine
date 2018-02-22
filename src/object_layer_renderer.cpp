#include "../include/object_layer_renderer.hpp"
#include "../include/object_layer.hpp"
#include "../include/map_object.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/utility.hpp"
#include <algorithm>
#include <limits>

void Object_Layer_Renderer::render(Map& map) {
    if (!layer.visible)
        return;

    batch.clear();

    // Casting the const away is fine since we're only sorting
    auto& object_layer =
        const_cast<Object_Layer&>(static_cast<const Object_Layer&>(layer));

    std::sort(object_layer.objects.begin(), object_layer.objects.end(),
        [](Map_Object* a, Map_Object* b) {
            float a_order, b_order;
            bool same_order = a->get_draw_order() == b->get_draw_order();
            if (a->get_draw_order() == Map_Object::NORMAL || same_order)
                a_order = a->get_real_position().y;
            else if (a->get_draw_order() == Map_Object::BELOW)
                a_order = std::numeric_limits<float>().min();
            else
                a_order = std::numeric_limits<float>().max();

            if (b->get_draw_order() == Map_Object::NORMAL || same_order)
                b_order = b->get_real_position().y;
            else if (b->get_draw_order() == Map_Object::BELOW)
                b_order = std::numeric_limits<float>().min();
            else
                b_order = std::numeric_limits<float>().max();

            return check_close(a_order, b_order) ?
                a->get_id() < b->get_id() : a_order < b_order;
    });

    for (auto& object : object_layer.objects) {
        object->render();
    }

    batch.draw(map.get_game().get_mvp(), map.get_game().ticks());
}
