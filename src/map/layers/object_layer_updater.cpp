#include "object_layer_updater.hpp"
#include "object_layer.hpp"
#include "../map_object.hpp"

void Object_Layer_Updater::update(Map&) {
    if (!layer.is_visible()) return;

    auto& object_layer = static_cast<Object_Layer&>(layer);
    auto& objects = object_layer.get_objects();
    for (auto& object : objects) {
        object->update();
    }
}
