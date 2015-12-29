#ifndef MAP_MAPPER_HPP
#define MAP_MAPPER_HPP

#include "../../include/editable.hpp"

class Map;

class Map_Mapper : public Property_Mapper {
public:
    Map_Mapper(Map* map);
    void populate(QtTreePropertyBrowser* browser,
            QtVariantPropertyManager* manager);
    void change_property(QtProperty* prop);
private:
    Map* map;
};

#endif // MAP_MAPPER_HPP
