#ifndef HPP_LAYER
#define HPP_LAYER

#include<memory>
#include <string>
#include "common.hpp"

class Layer_Renderer;
class Layer_Updater;

struct Layer {
    // Layer name
    std::string name;
    // Layer width (same as map width)
    int width;
    // Layer height (same as map height)
    int height;
    // Opacity value
    float opacity;
    // Is the layer visible?
    bool visible;
    // Layer properties
    Properties properties;
    // Layer rendering component
    std::unique_ptr<Layer_Renderer> renderer;
    // Layer logic component
    std::unique_ptr<Layer_Updater> updater;

    Layer();
    virtual ~Layer() = 0;
};

#endif
