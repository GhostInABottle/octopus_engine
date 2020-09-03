#ifndef HPP_UTILITY_STRING
#define HPP_UTILITY_STRING

#include <string>
#include <vector>

namespace string_utilities {
    // Trim a string from both sides
    void trim(std::string& s);
    std::string trim(const std::string& s);
    // Capitalize all characters in a string
    void capitalize(std::string& original);
    std::string capitalize(const std::string& original);
    // Check if two strings are equal (case insensitive)
    bool equal_strings(const std::string& str1, const std::string& str2);
    // Check if a string starts with another
    bool starts_with(const std::string& original, const std::string& ending);
    // Check if a string ends with another
    bool ends_with(const std::string& original, const std::string& ending);
    // Split a string to a vector
    std::vector<std::string> split(const std::string& original, const std::string& delims, bool compress = true);
    // Join vector elements into a string
    std::string join(const std::vector<std::string>& strings, const std::string& separator);
    // Convert "true"/"TRUE"/"1" to true and "false"/"FALSE"/"0" to false
    bool string_to_bool(const std::string& original);
}

#endif