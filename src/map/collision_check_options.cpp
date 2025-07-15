#include "collision_check_options.hpp"
#include "map_object.hpp"
#include "../utility/string.hpp"
#include "../configurations.hpp"

int Collision_Check_Options::proximity_distance = -1;

Collision_Check_Options::Collision_Check_Options(const Map_Object& object,
    Direction direction, Collision_Check_Type check_type,
    std::optional<xd::vec2> position, std::optional<float> speed)
    : object{ object }
    , direction{ direction }
    , check_type{ check_type }
    , position{ position.value_or(object.get_position()) }
    , speed{ speed.value_or(object.get_fps_independent_speed()) }
{
    if (proximity_distance == -1) {
        proximity_distance = Configurations::get<int>("player.proximity-distance");
    }
}
