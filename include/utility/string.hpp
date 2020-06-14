#ifndef HPP_UTILITY_STRING
#define HPP_UTILITY_STRING

#include <string>
#include <vector>
#include "../log.hpp"

// Trim a string from both sides
void trim(std::string& s);
std::string trim(const std::string& s);
// Capitalize all characters in a string
void capitalize(std::string& original);
std::string capitalize(const std::string& original);
// Check if two strings are equal (case insensitive)
bool equal_strings(const std::string& str1, const std::string& str2);
// Split a string to a vector
std::vector<std::string> split(const std::string& original, const std::string& delims, bool compress = true);
// Convert "true"/"TRUE"/"1" to true and "false"/"FALSE"/"0" to false
inline bool string_to_bool(const std::string& original) {
    auto capitalized = capitalize(original);
    if (capitalized == "TRUE" || capitalized == "1")
        return true;
    if (capitalized != "FALSE" && capitalized != "0")
        LOGGER_W << "Unable to convert string " << original << " to boolean. Defaulting to false.";
    return false;
}

#endif