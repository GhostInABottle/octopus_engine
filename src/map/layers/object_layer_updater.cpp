#include "../../../include/map/layers/object_layer.hpp"
#include "../../../include/map/layers/object_layer_updater.hpp"
#include "../../../include/map/map_object.hpp"

void Object_Layer_Updater::update(Map&) {
    if (!layer.is_visible()) return;

    auto& object_layer = static_cast<Object_Layer&>(layer);
    auto& objects = object_layer.get_objects();
    for (auto& object : objects) {
        object->update();
    }
}
