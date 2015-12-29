#ifndef OBJECT_LAYER_MAPPER_HPP
#define OBJECT_LAYER_MAPPER_HPP

#include "layer_mapper.hpp"

struct Object_Layer;

class Object_Layer_Mapper : public Layer_Mapper {
public:
    Object_Layer_Mapper(Object_Layer* layer);
    void populate(QtTreePropertyBrowser* browser,
            QtVariantPropertyManager* manager);
    void change_property(QtProperty* prop);
private:
    Object_Layer* layer;
};

#endif // OBJECT_LAYER_MAPPER_HPP

