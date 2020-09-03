#ifndef HPP_UTILITY_DIRECTION
#define HPP_UTILITY_DIRECTION

#include <iostream>
#include "../direction.hpp"
#include "string.hpp"

// Combine two directions
inline Direction operator|(Direction a, Direction b) {
    return static_cast<Direction>(static_cast<int>(a) | static_cast<int>(b));
}

// Check if a direction is set
inline Direction operator&(Direction a, Direction b) {
    return static_cast<Direction>(static_cast<int>(a) & static_cast<int>(b));
}

// Get the opposite direction
inline Direction opposite_direction(Direction dir) {
    int dir_int = static_cast<int>(dir);
    return static_cast<Direction>((dir_int + dir_int * 3) % 15);
}

// Convert a direction to a normalized 2D vector
inline xd::vec2 direction_to_vector(Direction dir) {
    float x = (dir & Direction::RIGHT) != Direction::NONE ?
        1.0f : (dir & Direction::LEFT) != Direction::NONE ? -1.0f : 0.0f;
    float y = (dir & Direction::DOWN) != Direction::NONE ?
        1.0f : (dir & Direction::UP) != Direction::NONE ? -1.0f : 0.0f;
    return xd::vec2(x, y);
}

// Convert a 2D vector to a direction
inline Direction vector_to_direction(xd::vec2 vec) {
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
        "",		    // 0
        "Up",		// 1
        "Right",	// 2
        "Up",		// 3 (Up + Right)
        "Down",		// 4
        "Up",		// 5 (Up + Down)
        "Down",		// 6 (Right + Down)
        "Up",		// 7 (Right + Up + Down)
        "Left",		// 8
        "Up",		// 9 (Left + Up)
        "Left",		// A (Left + Right)
        "Up",		// B (Left + Right + Up)
        "Down",		// C (Left + Down)
        "Up",		// D (Up + Down + Left)
        "Down",		// E (Down + Right + Left)
        "Down"		// All directions
    };
    int direction_int = static_cast<int>(dir);
    return direction_names[direction_int];
}

inline Direction string_to_direction(std::string str) {
    string_utilities::capitalize(str);
    if (str == "LEFT")
        return Direction::LEFT;
    else if (str == "RIGHT")
        return Direction::RIGHT;
    else if (str == "UP")
        return Direction::UP;
    else if (str == "DOWN")
        return Direction::DOWN;
    else
        return Direction::NONE;
}

// Get direction for object at pos1 to face pos2
inline Direction facing_direction(xd::vec2 pos1, xd::vec2 pos2, bool diagonal = false) {
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
inline Direction facing_direction(xd::ivec2 pos1, xd::ivec2 pos2) {
    return facing_direction(xd::vec2(pos1.x, pos1.y), xd::vec2(pos2.x, pos2.y));
}

inline bool is_diagonal(Direction dir) {
    Direction dir_minus_1 = static_cast<Direction>(static_cast<int>(dir) - 1);
    return (dir & dir_minus_1) != Direction::NONE;
}

inline std::ostream& operator<<(std::ostream& out, Direction dir)
{
    return out << direction_to_string(dir);
}

#endif
