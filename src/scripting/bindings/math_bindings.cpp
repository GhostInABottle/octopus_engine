#include "../../../include/configurations.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/utility/color.hpp"
#include "../../../include/xd/graphics/types.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <sstream>
#include <string>

void bind_math_types(sol::state& lua) {
    // 2D vector
    auto vec2_type = lua.new_usertype<xd::vec2>("Vec2",
        sol::call_constructor, sol::constructors<xd::vec2(), xd::vec2(const xd::vec2&), xd::vec2(float, float)>());
    vec2_type["x"] = &xd::vec2::x;
    vec2_type["y"] = &xd::vec2::y;
    vec2_type["length"] = [](xd::vec2& v) { return xd::length(v); };
    vec2_type["normal"] = [](xd::vec2& v) { return xd::normalize(v); };
    vec2_type[sol::meta_function::addition] = [](const xd::vec2& v1, const xd::vec2& v2) { return v1 + v2; };
    vec2_type[sol::meta_function::subtraction] = [](const xd::vec2& v1, const xd::vec2& v2) { return v1 - v2; };
    vec2_type[sol::meta_function::multiplication] = sol::overload(
        [](const xd::vec2& v1, float f) { return v1 * f; },
        [](float f, const xd::vec2& v1) { return f * v1; }
    );
    vec2_type[sol::meta_function::division] = sol::overload(
        [](const xd::vec2& v1, float f) { return v1 / f; },
        [](float f, const xd::vec2& v1) { return f / v1; }
    );
    vec2_type[sol::meta_function::to_string] = [](const xd::vec2& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(1);
        ss << "Vec2(" << val.x << ", " << val.y << ")";
        return ss.str();
    };

    // 3D vector
    auto vec3_type = lua.new_usertype<xd::vec3>("Vec3",
        sol::call_constructor, sol::constructors<xd::vec3(), xd::vec3(const xd::vec3&), xd::vec3(float, float, float)>());
    vec3_type["x"] = &xd::vec3::x;
    vec3_type["y"] = &xd::vec3::y;
    vec3_type["z"] = &xd::vec3::z;
    vec3_type["length"] = [](xd::vec3& v) { return xd::length(v); };
    vec3_type["normal"] = [](xd::vec3& v) { return xd::normalize(v); };
    vec3_type[sol::meta_function::addition] = [](const xd::vec3& v1, const xd::vec3& v2) { return v1 + v2; };
    vec3_type[sol::meta_function::subtraction] = [](const xd::vec3& v1, const xd::vec3& v2) { return v1 - v2; };
    vec3_type[sol::meta_function::multiplication] = sol::overload(
        [](const xd::vec3& v1, float f) { return v1 * f; },
        [](float f, const xd::vec3& v1) { return f * v1; }
    );
    vec3_type[sol::meta_function::division] = sol::overload(
        [](const xd::vec3& v1, float f) { return v1 / f; },
        [](float f, const xd::vec3& v1) { return f / v1; }
    );
    vec3_type[sol::meta_function::to_string] = [](const xd::vec3& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(1);
        ss << "Vec3(" << val.x << ", " << val.y << ", " << val.z << ")";
        return ss.str();
    };

    // 4D vector (or color)
    auto vec4_type = lua.new_usertype<xd::vec4>("Vec4",
        sol::call_constructor, sol::constructors<xd::vec4(), xd::vec4(const xd::vec4&), xd::vec4(float, float, float, float)>());
    vec4_type["x"] = &xd::vec4::x;
    vec4_type["y"] = &xd::vec4::y;
    vec4_type["z"] = &xd::vec4::z;
    vec4_type["w"] = &xd::vec4::w;
    vec4_type["r"] = &xd::vec4::r;
    vec4_type["g"] = &xd::vec4::g;
    vec4_type["b"] = &xd::vec4::b;
    vec4_type["a"] = &xd::vec4::a;
    vec4_type["length"] = [](const xd::vec4& v) { return xd::length(v); };
    vec4_type["normal"] = [](const xd::vec4& v) { return xd::normalize(v); };
    vec4_type[sol::meta_function::addition] = [](const xd::vec4& v1, const xd::vec4& v2) { return v1 + v2; };
    vec4_type[sol::meta_function::subtraction] = [](const xd::vec4& v1, const xd::vec4& v2) { return v1 - v2; };
    vec4_type[sol::meta_function::multiplication] = sol::overload(
        [](const xd::vec4& v1, float f) { return v1 * f; },
        [](float f, const xd::vec4& v1) { return f * v1; }
    );
    vec4_type[sol::meta_function::division] = sol::overload(
        [](const xd::vec4& v1, float f) { return v1 / f; },
        [](float f, const xd::vec4& v1) { return f / v1; }
    );
    vec4_type[sol::meta_function::to_string] = [](const xd::vec4& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(1);
        ss << "Vec4(" << val.x << ", " << val.y << ", " << val.z << ", " << val.w << ")";
        return ss.str();
    };
    vec4_type["to_hex"] = [](const xd::vec4& color) { return color_to_hex(color); };

    // Aliases for creating a color
    lua["Color"] = sol::overload(
        [](xd::vec4 other) { return xd::vec4{ other }; },
        [](float r, float g, float b, float a) { return xd::vec4{ r, g, b, a }; },
        [](float r, float g, float b) { return xd::vec4{ r, g, b, 1.0f }; },
        [](std::string name) { return string_to_color(name); }
    );

    // Rectangle
    auto rect_type = lua.new_usertype<xd::rect>("Rect",
        sol::call_constructor, sol::constructors<
        xd::rect(),
        xd::rect(const xd::rect&),
        xd::rect(float, float, float, float),
        xd::rect(const xd::vec2&, const xd::vec2&),
        xd::rect(const xd::vec2&, float, float),
        xd::rect(const xd::vec4&)>());
    rect_type["x"] = &xd::rect::x;
    rect_type["y"] = &xd::rect::y;
    rect_type["w"] = &xd::rect::w;
    rect_type["h"] = &xd::rect::h;
    rect_type["position"] = sol::property(sol::resolve<xd::vec2() const>(&xd::rect::position),
        sol::resolve<void(xd::vec2)>(&xd::rect::position));
    rect_type["size"] = sol::property(sol::resolve<xd::vec2() const>(&xd::rect::size),
        sol::resolve<void(xd::vec2)>(&xd::rect::size));
    rect_type["center"] = sol::property(&xd::rect::center);
    rect_type["intersects"] = &xd::rect::intersects;
    rect_type["touches"] = &xd::rect::touches;
    rect_type["contains"] = sol::overload(
        sol::resolve<bool(const xd::vec2&) const>(&xd::rect::contains),
        sol::resolve<bool(float, float) const>(&xd::rect::contains)
    );
    rect_type["move"] = &xd::rect::move;
    rect_type["extend"] = sol::overload(
        sol::resolve<xd::rect(xd::vec2) const>(&xd::rect::extend),
        sol::resolve<xd::rect(float) const>(&xd::rect::extend)
    );
    rect_type[sol::meta_function::to_string] = [](const xd::rect& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(0);
        ss << "Rect (" << val.x << ", " << val.y << ") " << val.w << "x" << val.h;
        return ss.str();
    };

    // Circle
    auto circle_type = lua.new_usertype<xd::circle>("Circle",
        sol::call_constructor, sol::constructors<
        xd::circle(),
        xd::circle(const xd::circle&),
        xd::circle(float, float, float),
        xd::circle(const xd::vec2&, float)>());
    circle_type["x"] = &xd::circle::x;
    circle_type["y"] = &xd::circle::y;
    circle_type["radius"] = &xd::circle::radius;
    circle_type["center"] = sol::property(sol::resolve<xd::vec2() const>(&xd::circle::center),
        sol::resolve<void(xd::vec2)>(&xd::circle::center));
    circle_type["intersects"] = sol::overload(
        sol::resolve<bool(const xd::circle&) const>(&xd::circle::intersects),
        sol::resolve<bool(const xd::rect&) const>(&xd::circle::intersects)
    );
    circle_type["touches"] = &xd::circle::touches;
    circle_type["contains"] = sol::overload(
        sol::resolve<bool(const xd::vec2&) const>(&xd::circle::contains),
        sol::resolve<bool(float, float) const>(&xd::circle::contains)
    );
    circle_type["move"] = &xd::circle::move;
    circle_type["extend"] = &xd::circle::extend;
    circle_type["to_rect"] = [](const xd::circle& c) { return static_cast<xd::rect>(c); };
    circle_type[sol::meta_function::to_string] = [](const xd::circle& val) {
        std::stringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(0);
        ss << "Circle (" << val.x << ", " << val.y << ") " << val.radius;
        return ss.str();
    };
}
