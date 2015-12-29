#ifndef MAP_OBJECT_MAPPER_HPP
#define MAP_OBJECT_MAPPER_HPP

#include "../../include/editable.hpp"

class Game;
class Map_Object;

class Map_Object_Mapper : public Property_Mapper {
public:
    Map_Object_Mapper(Game& game, Map_Object* object);
    void populate(QtTreePropertyBrowser* browser,
            QtVariantPropertyManager* manager);
    void change_property(QtProperty* prop);
private:
    Game& game;
    Map_Object* obj;
};

#endif // MAP_OBJECT_MAPPER_HPP
