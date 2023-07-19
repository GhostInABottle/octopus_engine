#ifndef LAYER_MAPPER_HPP
#define LAYER_MAPPER_HPP

#include "../../include/interfaces/editable.hpp"

struct Layer;

class Layer_Mapper : public Property_Mapper {
public:
    Layer_Mapper(Layer* layer);
    void populate(QtTreePropertyBrowser* browser,
            QtVariantPropertyManager* manager);
    void change_property(QtProperty* prop);
private:
    Layer* layer;
};

#endif // LAYER_MAPPER_HPP
