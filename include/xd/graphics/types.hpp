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
        constexpr bool intersects(const rect& other) const {
            return (
                    x < (other.x + other.w) &&
                (x+w) > other.x           &&
                    y < (other.y + other.h) &&
                (y+h) > other.y
            );
        }

        // check if the rect contains a point
        constexpr bool contains(const vec2& point) const {
            rect point_rect{ point, 0.0f, 0.0f };
            return intersects(point_rect) || touches(point_rect);
        }
        constexpr bool contains(float x, float y) const {
            return contains(vec2{ x, y });
        }

        // check if two rects touch
        constexpr bool touches(const rect& other) const {
            return !intersects(other) &&
                (
                    x <= (other.x + other.w) &&
                    (x + w) >= other.x &&
                    y <= (other.y + other.h) &&
                    (y + h) >= other.y
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

        constexpr vec2 center() const {
            return vec2{ x + w / 2, y + h / 2 };
        }

        float x, y, w, h;
    };

    struct circle {
        constexpr circle() noexcept
            : x(0.0f), y(0.0f), radius(0.0f) {}

        constexpr explicit circle(float x, float y, float radius) noexcept
            : x(x), y(y), radius(radius) {}

        constexpr explicit circle(const vec2& xy, float radius) noexcept
            : x(xy.x), y(xy.y), radius(radius) {}

        // conversion constructor
        template <typename T>
        constexpr explicit circle(const T& x, const T& y, const T& radius)
            : x(static_cast<float>(x))
            , y(static_cast<float>(y))
            , radius(static_cast<float>(radius)) {}

        // check if two circles intersect
        bool intersects(const circle& other) const {
            return glm::distance(center(), other.center()) < radius + other.radius;
        }

        // check if a circles and a rectangle intersect
        // based on: https://stackoverflow.com/a/402010
        bool intersects(const rect& rect) const {
            if (rect.contains(center())) return true;

            auto distance = xd::abs(center() - rect.center());

            auto half_size = rect.size() * 0.5f;
            auto too_far = distance.x >= (half_size.x + radius)
                || distance.y >= (half_size.y + radius);
            if (too_far) return false;

            auto close_enough = distance.x < half_size.x
                || distance.y < half_size.y;
            if (close_enough) return true;

            auto delta_x = distance.x - half_size.x;
            auto delta_y = distance.y - half_size.y;
            auto squared_distance = delta_x * delta_x + delta_y * delta_y;

            return squared_distance < radius * radius;
        }

        // check if a point falls inside the circle
        bool contains(const vec2& point) const {
            return glm::distance(center(), point) <= radius;
        }
        bool contains(float x, float y) const {
            return contains(vec2 { x, y });
        }
        bool contains_inside(const vec2& point) const {
            return glm::distance(center(), point) < radius;
        }
        bool contains_inside(float x, float y) const {
            return contains_inside(vec2{ x, y });
        }

        // check if two circles touch
        bool touches(const circle& other) const {
            auto distance = glm::distance(center(), other.center());
            return abs(distance - (radius + other.radius)) < 0.0001f;
        }

        // Check if two circles are equal
        constexpr bool operator==(const circle& other) const {
            return abs(x - other.x) < 0.0001f
                && abs(y - other.y) < 0.0001f
                && abs(radius - other.radius) < 0.0001f;
        }

        // Check if two circles are not equal
        constexpr bool operator!=(const circle& other) const {
            return !(*this == other);
        }

        constexpr vec2 center() const {
            return vec2{ x, y };
        }
        constexpr void center(vec2 new_pos) {
            x = new_pos.x;
            y = new_pos.y;
        }
        constexpr void move(vec2 displacement) {
            x += displacement.x;
            y += displacement.y;
        }

        explicit constexpr operator rect() const {
            return rect { x - radius, y - radius, radius * 2, radius * 2 };
        }

        float x, y, radius;
    };
}

#endif