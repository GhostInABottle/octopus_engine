#ifndef HPP_UTILITY
#define HPP_UTILITY

#include <string>
#include <vector>
#include <cmath>
#include "xd/glm.hpp"
#include "xd/graphics.hpp"
#include "vendor/rapidxml.hpp"

// Check if a file exists
bool file_exists(std::string filename);
// Read file content into a string
std::string read_file(std::string filename);
// Allocate an XML node
rapidxml::xml_node<>* xml_node(rapidxml::xml_document<>& doc,
        const std::string& name, const std::string& value = "",
        rapidxml::node_type type = rapidxml::node_element);
// Allocate an XML attribute
rapidxml::xml_attribute<>* xml_attribute(rapidxml::xml_document<>& doc,
        const std::string& name, const std::string& value);
// Trim a string from both sides
void trim(std::string& s);
// Capitalize all characters in a string
void capitalize(std::string& original);
// Change backslashes to forward slashes
void normalize_slashes(std::string& filename);
// Check if two strings are equal (case insensitive)
bool equal_strings(const std::string& str1, const std::string& str2);
// Split a string to a vector
std::vector<std::string> split(const std::string& original, const std::string& delims, bool compress = true);
// Return timestamp in "YYYY-MM-DD HH:MM:SS" format
std::string timestamp(bool date_only = false);
// Convert an unsigned int color value to a vec[4] of ARGB components
xd::vec4 int_to_color(unsigned int value);
// Convert a vec[4] of ARGB components to an unsigned int color value
unsigned int color_to_int(const xd::vec4& color);
// Convert a hex color into a vec[4] of ARGB components
xd::vec4 hex_to_color(std::string hex);
// Convert a vec4 color to a hex string
std::string color_to_hex(const xd::vec4& color);
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
