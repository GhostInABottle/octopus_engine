#ifndef HPP_COLLISION_CHECK_TYPES
#define HPP_COLLISION_CHECK_TYPES

// What type of collision check to perform
enum class Collision_Check_Types {
    NONE = 0,
    TILE = 1,
    OBJECT = 2,
    BOTH = 3,
    MULTI = 4,
    MULTI_OBJECTS = 6
};

inline Collision_Check_Types operator|(
        Collision_Check_Types a, Collision_Check_Types b) {
    return static_cast<Collision_Check_Types>(
        static_cast<int>(a) | static_cast<int>(b));
}
inline int operator&(Collision_Check_Types a, Collision_Check_Types b) {
    return static_cast<int>(a) & static_cast<int>(b);
}

#endif
