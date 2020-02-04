#ifndef HPP_UTILITY_MATH
#define HPP_UTILITY_MATH

#include <cmath>
#include <algorithm>
#include "../xd/glm.hpp"

// Linear interpolation between two floats at time alpha
inline float lerp(float start, float end, float alpha) {
    return (1.0f - alpha) * start + alpha * end;
}
// Linear interpolation between two 2D vectors at time alpha
xd::vec2 lerp(const xd::vec2& start, const xd::vec2& end, float alpha);
// Linear interpolation between two 3D vectors at time alpha
xd::vec3 lerp(const xd::vec3& start, const xd::vec3& end, float alpha);
// Linear interpolation between two 4D vectors at time alpha
xd::vec4 lerp(const xd::vec4& start, const xd::vec4& end, float alpha);
// Returns how much of a duration has passed
inline float calculate_alpha(long current, long start, long duration) {
    float alpha = 1.0f;
    if (duration != 0) {
        alpha = static_cast<float>(current - start) / duration;
        alpha = std::min(std::max(alpha, 0.0f), 1.0f);
    }
    return alpha;
}
// Check if two floating point numbers are close
inline bool check_close(float num1, float num2, float epsilon = 0.001f) {
    return std::fabs(num1 - num2) < epsilon;
}
// Check if two 2D vectors are are the same
inline bool check_close(xd::vec2 first, xd::vec2 second, float epsilon = 0.001f) {
    return check_close(first.x, second.x, epsilon)
        && check_close(first.y, second.y, epsilon);
}
// Check if two 3D vectors are are the same
inline bool check_close(xd::vec3 first, xd::vec3 second, float epsilon = 0.001f) {
    return check_close(first.x, second.x, epsilon)
        && check_close(first.y, second.y, epsilon)
        && check_close(first.z, second.z, epsilon);
}
// Check if two 4D vectors are are the same
inline bool check_close(xd::vec4 first, xd::vec4 second, float epsilon = 0.001f) {
    return check_close(first.x, second.x, epsilon)
        && check_close(first.y, second.y, epsilon)
        && check_close(first.z, second.z, epsilon)
        && check_close(first.w, second.w, epsilon);
}

#endif