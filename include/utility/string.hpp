#ifndef HPP_UTILITY_STRING
#define HPP_UTILITY_STRING

#include <string>
#include <vector>

// Trim a string from both sides
void trim(std::string& s);
// Capitalize all characters in a string
void capitalize(std::string& original);
// Check if two strings are equal (case insensitive)
bool equal_strings(const std::string& str1, const std::string& str2);
// Split a string to a vector
std::vector<std::string> split(const std::string& original, const std::string& delims, bool compress = true);
// Return timestamp in "YYYY-MM-DD HH:MM:SS" format
std::string timestamp(bool date_only = false);

#endif