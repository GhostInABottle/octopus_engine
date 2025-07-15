#ifndef HPP_UTILITY_COLOR
#define HPP_UTILITY_COLOR

#include <string>
#include "../xd/glm.hpp"

// Convert an unsigned int color value to a vec[4] of ARGB components
xd::vec4 int_to_color(unsigned int value);
// Convert a vec[4] of ARGB components to an unsigned int color value
unsigned int color_to_int(const xd::vec4& color);
// Convert a hex color into a vec[4] of ARGB components
xd::vec4 hex_to_color(std::string hex);
// Convert a vec4 color to a hex string
std::string color_to_hex(const xd::vec4& color, bool always_include_alpha = false);
// Convert a vec4 color to an RGBA string
std::string color_to_rgba_string(const xd::vec4& color);
// Convert a named color or hex value into a vec4 color
xd::vec4 string_to_color(std::string name);

#endif
