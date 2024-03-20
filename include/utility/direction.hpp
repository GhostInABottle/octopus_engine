#ifndef HPP_UTILITY_DIRECTION
#define HPP_UTILITY_DIRECTION

#include "../log.hpp"
#include "../direction.hpp"
#include "../xd/glm.hpp"
#include "string.hpp"

// Combine two directions
constexpr Direction operator|(Direction a, Direction b) noexcept {
    return static_cast<Direction>(static_cast<int>(a) | static_cast<int>(b));
}

// Check if a direction is set
constexpr Direction operator&(Direction a, Direction b) noexcept {
    return static_cast<Direction>(static_cast<int>(a) & static_cast<int>(b));
}

// Check if a direction contains a component
constexpr bool direction_contains(Direction dir, Direction component) {
    return (dir & component) != Direction::NONE;
}

// Get the opposite direction
constexpr Direction opposite_direction(Direction dir) noexcept {
    const int dir_int = static_cast<int>(dir);
    return static_cast<Direction>((dir_int + dir_int * 3) % 15);
}

// Check if it's a diagonal direction
constexpr bool is_diagonal(Direction dir) noexcept {
    const Direction dir_minus_1 = static_cast<Direction>(static_cast<int>(dir) - 1);
    return direction_contains(dir, dir_minus_1);
}

// Check if the direction is relative to the current one (forward or backward)
constexpr bool is_relative_direction(Direction dir) noexcept {
    return direction_contains(dir, Direction::FORWARD)
        || direction_contains(dir, Direction::BACKWARD);
}

// Convert a direction to a normalized 2D vector
constexpr xd::vec2 direction_to_vector(Direction dir) noexcept {
    const float x = direction_contains(dir, Direction::RIGHT) ?
        1.0f : direction_contains(dir, Direction::LEFT) ? -1.0f : 0.0f;
    const float y = direction_contains(dir, Direction::DOWN) ?
        1.0f : direction_contains(dir, Direction::UP) ? -1.0f : 0.0f;
    const xd::vec2 result{x, y};

    if (is_diagonal(dir)) {
        const float one_over_sqrt_2 = 0.70710678118f;
        return result * one_over_sqrt_2;
    }

    return result;
}

// Convert a 2D vector to a direction
constexpr Direction vector_to_direction(xd::vec2 vec) noexcept {
    auto dir = Direction::NONE;
    if (vec.x > 0)
        dir = dir | Direction::RIGHT;
    if (vec.x < 0)
        dir = dir | Direction::LEFT;
    if (vec.y > 0)
        dir = dir | Direction::DOWN;
    if (vec.y < 0)
        dir = dir | Direction::UP;
    return dir;
}

// Get the name of a direction
inline std::string direction_to_string(Direction dir) {
    static std::string direction_names[] = {
        "",                     // 0
        "Up",                   // 1
        "Right",                // 2
        "Up|Right",             // 3
        "Down",                 // 4
        "Up|Down",              // 5
        "Down|Right",           // 6
        "Up|Down|Right",        // 7
        "Left",                 // 8
        "Up|Left",              // 9
        "Left|Right",           // A
        "Up|Left|Right",        // B
        "Down|Left",            // C
        "Up|Down|Left",         // D
        "Down|Left|Right",      // E
        "Up|Down|Left|Right"    // F
    };
    const int direction_int = static_cast<int>(dir);
    return direction_names[direction_int];
}

// Map a direction name to a direction
inline Direction string_to_direction(std::string str) {
    auto dir = Direction::NONE;
    if (str.empty()) return dir;

    string_utilities::capitalize(str);
    auto parts = string_utilities::split(str, "|");
    for (auto& part : parts) {
        if (part == "UP")
            dir = dir | Direction::UP;
        else if (part == "DOWN")
            dir = dir | Direction::DOWN;
        else if (part == "LEFT")
            dir = dir | Direction::LEFT;
        else if (part == "RIGHT")
            dir = dir | Direction::RIGHT;
        else {
            LOGGER_W << "Unexpected direction " << part << " in directional string " << str;
        }

    }
    return dir;
}

// Get direction for object at pos1 to face pos2
constexpr Direction facing_direction(xd::vec2 pos1, xd::vec2 pos2, bool diagonal = false) noexcept {
    float x_change = pos2.x - pos1.x;
    float y_change = pos2.y - pos1.y;
    if (!diagonal) {
        // We only care about the direction with the least distance
        if (std::abs(y_change) > 0.0f && std::abs(y_change) > std::abs(x_change))
            x_change = 0.0f;
        else if (std::abs(x_change) > 0.0f && std::abs(x_change) > std::abs(y_change))
            y_change = 0.0f;
    }

    Direction direction = Direction::NONE;
    if (diagonal) {
        if (x_change > 0.0f)
            direction = direction | Direction::RIGHT;
        if (x_change < 0.0f)
            direction = direction | Direction::LEFT;
        if (y_change > 0.0f)
            direction = direction | Direction::DOWN;
        if (y_change < 0.0f)
            direction = direction | Direction::UP;
    } else {
        if (x_change > 0.0f)
            direction = Direction::RIGHT;
        else if (x_change < 0.0f)
            direction = Direction::LEFT;
        else if (y_change > 0.0f)
            direction = Direction::DOWN;
        else if (y_change < 0.0f)
            direction = Direction::UP;
    }

    return direction;
}

constexpr Direction facing_direction(xd::ivec2 pos1, xd::ivec2 pos2) {
    return facing_direction(xd::vec2(pos1.x, pos1.y), xd::vec2(pos2.x, pos2.y));
}

constexpr Direction diagonal_to_four_directions(Direction dir) {
    if (direction_contains(dir, Direction::UP))
        return Direction::UP;
    else if (direction_contains(dir, Direction::DOWN))
        return Direction::DOWN;
    else if (direction_contains(dir, Direction::LEFT))
        return Direction::LEFT;
    else if (direction_contains(dir, Direction::RIGHT))
        return Direction::RIGHT;
    else
        return Direction::NONE;
}

inline std::ostream& operator<<(std::ostream& out, Direction dir)
{
    return out << direction_to_string(dir);
}

#endif
