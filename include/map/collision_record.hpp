#ifndef HPP_COLLISION_RECORD
#define HPP_COLLISION_RECORD

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
    // The closest object that can be triggered, if any
    Map_Object* proximate_object;

    // Does collision type allow passing through?
    bool passable() const noexcept {
        return type == Collision_Type::NONE || type == Collision_Type::AREA;
    }
    Collision_Record(Collision_Type type = Collision_Type::NONE) noexcept
        : type(type)
        , this_object(nullptr)
        , other_object(nullptr)
        , other_area(nullptr)
        , proximate_object(nullptr) {}
};

#endif
