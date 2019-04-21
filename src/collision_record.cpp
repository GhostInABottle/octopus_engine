#include "../include/collision_record.hpp"

Collision_Record::Collision_Record(Collision_Types type,
    const Map_Object* this_object, Map_Object* other_object) :
        type(type), this_object(this_object), other_object(other_object),
        edge_direction(Direction::NONE)  {}

void Collision_Record::set(Collision_Types new_type, Map_Object* new_other_object) {
    type = new_type;
    other_object = new_other_object;
}
