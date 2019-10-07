#include "../include/object_layer_updater.hpp"
#include "../include/object_layer.hpp"
#include "../include/map_object.hpp"

void Object_Layer_Updater::update(Map&) {
    if (!layer.visible)
        return;
    auto& object_layer = static_cast<Object_Layer&>(layer);
    for (auto& object : object_layer.objects) {
        object->update();
    }
}
