#include "object_layer_renderer.hpp"
#include "object_layer.hpp"
#include "../map.hpp"
#include "../map_object.hpp"
#include "../../camera.hpp"
#include "../../configurations.hpp"
#include "../../game.hpp"
#include "../../utility/color.hpp"
#include "../../utility/math.hpp"
#include <algorithm>
#include <limits>

Object_Layer_Renderer::Object_Layer_Renderer(const Layer& layer, const Camera& camera)
        : Layer_Renderer(layer, camera) {
    auto outline_color_config = Configurations::get<std::string>("game.object-outline-color");
    default_outline_color = string_to_color(outline_color_config);
    batch.set_outline_color(default_outline_color);
}

void Object_Layer_Renderer::render(Map& map) {
    batch.clear();

    // Casting the const away is fine since we're only sorting
    auto& object_layer = const_cast<Object_Layer&>(static_cast<const Object_Layer&>(layer));

    auto& objects = object_layer.get_objects();
    std::sort(objects.begin(), objects.end(),
        [](Map_Object* a, Map_Object* b) {
            float a_order, b_order;
            bool same_order = a->get_draw_order() == b->get_draw_order();
            if (a->get_draw_order() == Map_Object::Draw_Order::NORMAL || same_order)
                a_order = a->get_real_position().y;
            else if (a->get_draw_order() == Map_Object::Draw_Order::BELOW)
                a_order = std::numeric_limits<float>().lowest();
            else
                a_order = std::numeric_limits<float>().max();

            if (b->get_draw_order() == Map_Object::Draw_Order::NORMAL || same_order)
                b_order = b->get_real_position().y;
            else if (b->get_draw_order() == Map_Object::Draw_Order::BELOW)
                b_order = std::numeric_limits<float>().lowest();
            else
                b_order = std::numeric_limits<float>().max();

            return check_close(a_order, b_order) ?
                a->get_id() < b->get_id() : a_order < b_order;
    });

    bool draw_outlines = map.get_draw_outlines();
    const auto& mvp = camera.get_mvp();

    for (auto& object : objects) {
        if (draw_outlines && object->is_outlined()) {
            auto color = object->get_outline_color();
            batch.set_outline_color(color.value_or(default_outline_color));
            batch.draw(mvp);
            batch.clear();
            object->render();
            batch.draw_outlined(mvp);
            batch.clear();
        } else {
            object->render();
        }
    }

    batch.draw(mvp);
}
