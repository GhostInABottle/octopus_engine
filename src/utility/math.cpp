#include "math.hpp"

xd::vec2 lerp(const xd::vec2& start, const xd::vec2& end, float alpha) noexcept {
    xd::vec2 result;
    result.x = lerp(start.x, end.x, alpha);
    result.y = lerp(start.y, end.y, alpha);
    return result;
}

xd::vec3 lerp(const xd::vec3& start, const xd::vec3& end, float alpha) noexcept {
    xd::vec3 result;
    result.x = lerp(start.x, end.x, alpha);
    result.y = lerp(start.y, end.y, alpha);
    result.z = lerp(start.z, end.z, alpha);
    return result;
}

xd::vec4 lerp(const xd::vec4& start, const xd::vec4& end, float alpha) noexcept {
    xd::vec4 result;
    result.x = lerp(start.x, end.x, alpha);
    result.y = lerp(start.y, end.y, alpha);
    result.z = lerp(start.z, end.z, alpha);
    result.w = lerp(start.w, end.w, alpha);
    return result;
}
