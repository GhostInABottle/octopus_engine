#ifndef HPP_OBJECT_LAYER_UPDATER
#define HPP_OBJECT_LAYER_UPDATER

#include "layer_updater.hpp"

class Object_Layer_Updater : public Layer_Updater {
public:
    Object_Layer_Updater(Layer* layer) : Layer_Updater(layer) {}
    void update(Map& map);
};

#endif
