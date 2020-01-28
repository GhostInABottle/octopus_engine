#include "../include/collision_record.hpp"

Collision_Record::Collision_Record(Collision_Types type,
    const Map_Object* this_object, Map_Object* other_object,
    Map_Object* other_area) :
        type(type), this_object(this_object),
        other_object(other_object), other_area(other_area),
        edge_direction(Direction::NONE)  {}

