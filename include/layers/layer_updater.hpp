#ifndef HPP_LAYER_UPDATER
#define HPP_LAYER_UPDATER

#include "../xd/entity.hpp"
#include "../map.hpp"

class Layer_Updater : public xd::logic_component<Map> {
public:
    Layer_Updater(Layer& layer) noexcept : layer(layer) {}
    virtual void update(Map& map) = 0;
protected:
    Layer& layer;
};

#endif
