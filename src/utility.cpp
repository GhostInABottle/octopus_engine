#include "../include/utility.hpp"
#include <stdexcept>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/string.hpp>

bool file_exists(const std::string& filename) {
    std::ifstream stream(normalize_slashes(filename));
    return static_cast<bool>(stream);
}

std::string read_file(const std::string& filename) {
    std::ifstream stream(normalize_slashes(filename));
    if (!stream)
        throw std::runtime_error("Couldn't read file " + filename);
    return std::string((std::istreambuf_iterator<char>(stream)),
                 std::istreambuf_iterator<char>());
}

rapidxml::xml_node<>* xml_node(rapidxml::xml_document<>& doc,
        const std::string& name, const std::string& value,
        rapidxml::node_type type) {
    char* name_str = doc.allocate_string(name.c_str());
    char* value_str = nullptr;
    if (!value.empty())
        value_str = doc.allocate_string(value.c_str());
    return doc.allocate_node(type, name_str, value_str);
}

rapidxml::xml_attribute<>* xml_attribute(rapidxml::xml_document<>& doc,
        const std::string& name, const std::string& value) {
    char* name_str = doc.allocate_string(name.c_str());
    char* value_str = doc.allocate_string(value.c_str());
    return doc.allocate_attribute(name_str, value_str);
}

std::string trim(std::string s) {
    boost::trim(s);
    return s;
}

std::vector<std::string> split(const std::string& original, const std::string& delims, bool compress) {
    std::vector<std::string> elements;
    auto token_compress = compress ? boost::token_compress_on : boost::token_compress_off;
    if (!original.empty()) {
        // Ignore unsafe function warning
        #pragma warning(push)
        #pragma warning(disable: 4996)
        boost::split(elements, original, boost::is_any_of(delims), token_compress);
        #pragma warning(pop)
    }
    return elements;
}


std::string timestamp(bool date_only) {
    std::time_t seconds_time = std::time(0);
    // Ignore unsafe function warning
    #pragma warning(push)
    #pragma warning(disable: 4996)
    std::tm local_time = *std::localtime(&seconds_time);
    #pragma warning(pop)

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << 1900 + local_time.tm_year << "-";
    oss << std::setw(2) << std::setfill('0') << local_time.tm_mon + 1 << "-";
    oss << std::setw(2) << std::setfill('0') << local_time.tm_mday;
    if (!date_only) {
        oss << " ";
        oss << std::setw(2) << std::setfill('0') << local_time.tm_hour << ":";
        oss << std::setw(2) << std::setfill('0') << local_time.tm_min << ":";
        oss << std::setw(2) << std::setfill('0') << local_time.tm_sec;
    }

    return oss.str();
}

std::string capitalize(std::string original) {
    boost::to_upper(original);
    return original;
}

std::string normalize_slashes(std::string filename) {
    boost::replace_all(filename, "\\", "/");
    return filename;
}

bool equal_strings(const std::string& str1, const std::string& str2) {
    return boost::iequals(str1, str2);
}

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

xd::vec2 lerp(const xd::vec2& start, const xd::vec2& end, float alpha) {
    xd::vec2 result;
    result.x = lerp(start.x, end.x, alpha);
    result.y = lerp(start.y, end.y, alpha);
    return result;
}

xd::vec3 lerp(const xd::vec3& start, const xd::vec3& end, float alpha) {
    xd::vec3 result;
    result.x = lerp(start.x, end.x, alpha);
    result.y = lerp(start.y, end.y, alpha);
    result.z = lerp(start.z, end.z, alpha);
    return result;
}

xd::vec4 lerp(const xd::vec4& start, const xd::vec4& end, float alpha) {
    xd::vec4 result;
    result.x = lerp(start.x, end.x, alpha);
    result.y = lerp(start.y, end.y, alpha);
    result.z = lerp(start.z, end.z, alpha);
    result.w = lerp(start.w, end.w, alpha);
    return result;
}
