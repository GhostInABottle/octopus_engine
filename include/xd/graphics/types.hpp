#ifndef H_XD_GRAPHICS_TYPES
#define H_XD_GRAPHICS_TYPES

#include "../glm.hpp"

namespace xd
{
    struct rect
    {
        rect() noexcept
            : x(0), y(0), w(0), h(0)
        {}

        explicit rect(float x, float y, float w, float h) noexcept
            : x(x), y(y), w(w), h(h)
        {}

        explicit rect(const glm::vec2& xy, const glm::vec2& wh) noexcept
            : x(xy.x), y(xy.y), w(wh.x), h(wh.y)
        {}

        explicit rect(const glm::vec2& xy, float w, float h) noexcept
            : x(xy.x), y(xy.y), w(w), h(h)
        {}

        explicit rect(const glm::vec4& rect)
            : x(rect[0]), y(rect[1]), w(rect[2]), h(rect[3])
        {}

        // conversion constructor
        template <typename T>
        explicit rect (const T& x, const T& y, const T& w, const T& h)
            : x(static_cast<float>(x))
            , y(static_cast<float>(y))
            , w(static_cast<float>(w))
            , h(static_cast<float>(h))
        {}

        // check if two rects intersect
        bool intersects(const rect& other) noexcept {
            return (
                    x < (other.x + other.w) &&
                (x+w) > other.x           &&
                    y < (other.y + other.h) &&
                (y+h) > other.y
            );
        }

        // Check if two rects are equal
        bool operator==(const rect& other) const {
            return xd::abs(x - other.x) < 0.0001f
                && xd::abs(y - other.y) < 0.0001f
                && xd::abs(w - other.w) < 0.0001f
                && xd::abs(h - other.h) < 0.0001f;
        }

        // Check if two rects are not equal
        bool operator!=(const rect& other) const {
            return !(*this == other);
        }

        xd::vec2 position() const {
            return xd::vec2{ x, y };
        }
        void position(xd::vec2 new_pos) {
            x = new_pos.x;
            y = new_pos.y;
        }

        xd::vec2 size() const {
            return xd::vec2{ w, h };
        }
        void size(xd::vec2 new_size) {
            w = new_size.x;
            h = new_size.y;
        }

        float x, y, w, h;
    };
}

#endif