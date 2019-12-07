#include "../include/object_layer_renderer.hpp"
#include "../include/object_layer.hpp"
#include "../include/map_object.hpp"
#include "../include/map.hpp"
#include "../include/game.hpp"
#include "../include/utility.hpp"
#include "../include/camera.hpp"
#include "../include/configurations.hpp"
#include <algorithm>
#include <limits>

Object_Layer_Renderer::Object_Layer_Renderer(const Layer& layer, const Camera& camera)
        : Layer_Renderer(layer, camera) {
    default_outline_color = hex_to_color(Configurations::get<std::string>("game.object-outline-color"));
    batch.set_outline_color(default_outline_color);
}

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
            if (a->get_draw_order() == Map_Object::Draw_Order::NORMAL || same_order)
                a_order = a->get_real_position().y;
            else if (a->get_draw_order() == Map_Object::Draw_Order::BELOW)
                a_order = std::numeric_limits<float>().min();
            else
                a_order = std::numeric_limits<float>().max();

            if (b->get_draw_order() == Map_Object::Draw_Order::NORMAL || same_order)
                b_order = b->get_real_position().y;
            else if (b->get_draw_order() == Map_Object::Draw_Order::BELOW)
                b_order = std::numeric_limits<float>().min();
            else
                b_order = std::numeric_limits<float>().max();

            return check_close(a_order, b_order) ?
                a->get_id() < b->get_id() : a_order < b_order;
    });

    for (auto& object : object_layer.objects) {
        if (object->is_outlined()) {
            auto color = object->get_outline_color();
            batch.set_outline_color(color.value_or(default_outline_color));
            batch.draw(camera.get_mvp(), map.get_game().ticks());
            batch.clear();
            object->render();
            batch.draw_outlined(camera.get_mvp(), map.get_game().ticks());
            batch.clear();
        } else {
            object->render();
        }
    }

    batch.draw(camera.get_mvp(), map.get_game().ticks());
}
