#ifndef HPP_DIRECTION
#define HPP_DIRECTION

// Movement directions
enum class Direction { NONE = 0, UP = 1, RIGHT = 2, DOWN = 4, LEFT = 8, FORWARD = 16, BACKWARD = 32 };

// Combine two directions
constexpr Direction operator|(Direction a, Direction b) noexcept {
    return static_cast<Direction>(static_cast<int>(a) | static_cast<int>(b));
}

// Check if a direction is set
constexpr Direction operator&(Direction a, Direction b) noexcept {
    return static_cast<Direction>(static_cast<int>(a) & static_cast<int>(b));
}

#endif
