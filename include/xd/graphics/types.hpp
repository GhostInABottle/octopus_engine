#ifndef H_XD_GRAPHICS_TYPES
#define H_XD_GRAPHICS_TYPES

#include "../glm.hpp"

namespace xd
{
    struct rect
    {
        constexpr rect() noexcept
            : x(0.0f), y(0.0f), w(0.0f), h(0.0f)
        {}

        constexpr explicit rect(float x, float y, float w, float h) noexcept
            : x(x), y(y), w(w), h(h)
        {}

        constexpr explicit rect(const vec2& xy, const vec2& wh) noexcept
            : x(xy.x), y(xy.y), w(wh.x), h(wh.y)
        {}

        constexpr explicit rect(const vec2& xy, float w, float h) noexcept
            : x(xy.x), y(xy.y), w(w), h(h)
        {}

        constexpr explicit rect(const vec4& rect)
            : x(rect[0]), y(rect[1]), w(rect[2]), h(rect[3])
        {}

        // conversion constructor
        template <typename T>
        constexpr explicit rect (const T& x, const T& y, const T& w, const T& h)
            : x(static_cast<float>(x))
            , y(static_cast<float>(y))
            , w(static_cast<float>(w))
            , h(static_cast<float>(h))
        {}

        // check if two rects intersect
        constexpr bool intersects(const rect& other) noexcept {
            return (
                    x < (other.x + other.w) &&
                (x+w) > other.x           &&
                    y < (other.y + other.h) &&
                (y+h) > other.y
            );
        }

        // Check if two rects are equal
        constexpr bool operator==(const rect& other) const {
            return abs(x - other.x) < 0.0001f
                && abs(y - other.y) < 0.0001f
                && abs(w - other.w) < 0.0001f
                && abs(h - other.h) < 0.0001f;
        }

        // Check if two rects are not equal
        constexpr bool operator!=(const rect& other) const {
            return !(*this == other);
        }

        constexpr vec2 position() const {
            return vec2{ x, y };
        }
        constexpr void position(vec2 new_pos) {
            x = new_pos.x;
            y = new_pos.y;
        }
        constexpr void move(vec2 displacement) {
            x += displacement.x;
            y += displacement.y;
        }

        constexpr vec2 size() const {
            return vec2{ w, h };
        }
        constexpr void size(vec2 new_size) {
            w = new_size.x;
            h = new_size.y;
        }

        float x, y, w, h;
    };
}

#endif