#ifndef HPP_IMAGE_LAYER_UPDATER
#define HPP_IMAGE_LAYER_UPDATER

#include "layer_updater.hpp"

class Image_Layer_Updater : public Layer_Updater {
public:
    explicit Image_Layer_Updater(Layer* layer) : Layer_Updater(layer) {}
    void update(Map& map) override;
};

#endif
