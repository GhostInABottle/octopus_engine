#include "../include/collision_record.hpp"

Collision_Record::Collision_Record(Collision_Types type, 
    const Map_Object* this_object, Map_Object* other_object) :
        type(type), this_object(this_object), other_object(other_object),
        edge_direction(Direction::NONE)  {}

void Collision_Record::set(Collision_Types type, Map_Object* other_object) {
    this->type = type;
    this->other_object = other_object;
}
