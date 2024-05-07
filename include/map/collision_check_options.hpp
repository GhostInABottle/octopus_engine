#ifndef HPP_COLLISION_CHECK_OPTIONS
#define HPP_COLLISION_CHECK_OPTIONS

#include "../direction.hpp"
#include "../xd/glm.hpp"
#include "collision_check_types.hpp"
#include <optional>
#include <string>

class Map_Object;

// Options passed to check collision
struct Collision_Check_Options {
    const Map_Object& object;
    Direction direction;
    Collision_Check_Type check_type;
    xd::vec2 position;
    float speed;
    static int proximity_distance;

    Collision_Check_Options(const Map_Object& object, Direction direction,
        Collision_Check_Type check_type = Collision_Check_Type::BOTH,
        std::optional<xd::vec2> position = std::nullopt,
        std::optional<float> speed = std::nullopt);
};

#endif
