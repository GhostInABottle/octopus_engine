#ifndef HPP_COLLISION_CHECK_TYPES
#define HPP_COLLISION_CHECK_TYPES

// What type of collision check to perform
enum class Collision_Check_Type {
    NONE = 0,
    TILE = 1,
    OBJECT = 2,
    BOTH = 3,
};

inline Collision_Check_Type operator|(
        Collision_Check_Type a, Collision_Check_Type b) noexcept {
    return static_cast<Collision_Check_Type>(
        static_cast<int>(a) | static_cast<int>(b));
}
inline int operator&(Collision_Check_Type a, Collision_Check_Type b) noexcept {
    return static_cast<int>(a) & static_cast<int>(b);
}

#endif
