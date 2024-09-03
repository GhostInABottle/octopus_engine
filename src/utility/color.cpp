#include "../../include/utility/color.hpp"
#include "../../include/configurations.hpp"
#include <stdexcept>
#include <sstream>

xd::vec4 int_to_color(unsigned int value) {
    xd::vec4 color;
    color.a = ((value >> 24) & 0xff) / 255.0f;
    color.r = ((value >> 16) & 0xff) / 255.0f;
    color.g = ((value >> 8) & 0xff) / 255.0f;
    color.b = (value & 0xff) / 255.0f;
    return color;
}

unsigned int color_to_int(const xd::vec4& color) {
    unsigned int value = 0;
    value = static_cast<unsigned int>(color.b * 255.0f);
    value |= static_cast<unsigned int>(color.g * 255.0f) << 8;
    value |= static_cast<unsigned int>(color.r * 255.0f) << 16;
    value |= static_cast<unsigned int>(color.a * 255.0f) << 24;
    return value;
}

xd::vec4 hex_to_color(std::string hex) {
    if (hex.empty()) {
        throw std::runtime_error("Empty hex string");
    }
    if (hex[0] == '#') {
        hex = hex.substr(1);
    }
    if (hex.size() > 8) {
        throw std::runtime_error(hex + " is too long");
    }

    unsigned int value;
    std::stringstream ss(hex);
    ss >> std::hex >> value;
    if (!ss) {
        throw std::runtime_error("Invalid hex string");
    }

    if (hex.size() == 6) {
        value |= 0xff000000;
    }

    return int_to_color(value);
}

std::string color_to_hex(const xd::vec4& color, bool always_include_alpha) {
    int value = color_to_int(color);
    std::stringstream ss;
    ss << std::hex << value;
    auto hex_string = ss.str();
    if (!always_include_alpha && hex_string.size() == 8 && hex_string.find("ff") == 0) {
        return hex_string.substr(2);
    }
    return hex_string;
}

std::string color_to_rgba_string(const xd::vec4& color) {
    std::stringstream ss;
    ss << static_cast<int>(color.r * 255) << ','
        << static_cast<int>(color.g * 255) << ','
        << static_cast<int>(color.b * 255) << ','
        << static_cast<int>(color.a * 255);
    return ss.str();
}

// Convert a named color or hex value into a vec4 color
xd::vec4 string_to_color(std::string name) {
    if (name == "clear") {
        auto clear = hex_to_color(Configurations::get<std::string>("startup.clear-color"));
        return xd::vec4{ clear };
    } else if (name == "none") {
        return xd::vec4{};
    } else if (name == "black") {
        return xd::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
    } else if (name == "red") {
        return xd::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };
    } else if (name == "green") {
        return xd::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
    } else if (name == "blue") {
        return xd::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };
    } else if (name == "yellow") {
        return xd::vec4{ 1.0f, 1.0f, 0.0f, 1.0f };
    } else if (name == "white") {
        return xd::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
    } else if (name == "gray") {
        return xd::vec4{ 0.5f, 0.5f, 0.5f, 1.0f };
    } else if (name == "purple") {
        return xd::vec4{ 1.0f, 0.0f, 1.0f, 1.0f };
    } else if (name == "cyan") {
        return xd::vec4{ 0.0f, 1.0f, 1.0f, 1.0f };
    }

    // Throws if name is not a valid hex
    return hex_to_color(name);
}