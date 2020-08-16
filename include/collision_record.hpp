#ifndef HPP_COLLISION_RECORD
#define HPP_COLLISION_RECORD

#include <string>
#include <unordered_map>
#include "direction.hpp"

class Map_Object;

// Types of collision events
enum class Collision_Type {
    NONE,           // No collisions found
    NO_MOVE,        // No collisions because this object didn't move
    TILE,           // Tile collision
    OBJECT,         // Object collision: impassable, trigger on key press
    AREA,           // Area collision: passable
};

// Structure returned after collision detection
struct Collision_Record {
    // What triggered this collision
    Collision_Type type;
    // The object that triggered it (usually the player)
    const Map_Object* this_object;
    // The first other object, if any
    Map_Object* other_object;
    // The first collision area
    Map_Object* other_area;
    // Map of all objects we collided with
    std::unordered_map<std::string, Map_Object*> other_objects;
    // Map of all areas we collided with
    std::unordered_map<std::string, Map_Object*> other_areas;
    // Direction of edge tiles (e.g. correcting tiles next to a door)
    Direction edge_direction;
    // Does collision type allow passing through?
    bool passable() const {
        return type == Collision_Type::NONE || type == Collision_Type::AREA;
    }
    Collision_Record(Collision_Type type = Collision_Type::NONE,
        const Map_Object* this_object = nullptr,
        Map_Object* other_object = nullptr,
        Map_Object* other_area = nullptr);
};

#endif
