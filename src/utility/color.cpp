#include "../../include/utility/color.hpp"
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
    if (hex.empty())
        throw std::runtime_error("Empty hex string");
    if (hex[0] == '#')
        hex = hex.substr(1);
    if (hex.size() > 8)
        throw std::runtime_error(hex + " is too long");
    std::stringstream ss(hex);
    unsigned int value;
    ss >> std::hex >> value;
    if (!ss)
        throw std::runtime_error("Invalid hex string");
    if (hex.size() == 6)
        value |= 0xff000000;
    return int_to_color(value);
}

std::string color_to_hex(const xd::vec4& color) {
    int value = color_to_int(color);
    std::stringstream ss;
    ss << std::hex << value;
    return ss.str();
}