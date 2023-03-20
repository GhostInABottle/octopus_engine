#include "../../include/utility/string.hpp"
#include "../../include/log.hpp"
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <algorithm>

void string_utilities::trim(std::string& s) {
    boost::trim(s);
}

std::string string_utilities::trim(const std::string& s) {
    std::string copy{s};
    trim(copy);
    return copy;
}

void string_utilities::capitalize(std::string& original) {
    std::transform(original.begin(), original.end(), original.begin(),
        [](unsigned char c) { return std::toupper(c); }
    );
}

std::string string_utilities::capitalize(const std::string& original) {
    std::string copy{original};
    capitalize(copy);
    return copy;
}

bool string_utilities::equal_strings(const std::string& str1, const std::string& str2) {
    return boost::iequals(str1, str2);
}

bool string_utilities::starts_with(const std::string& original, const std::string& start) {
    return boost::starts_with(original, start);
}

bool string_utilities::ends_with(const std::string& original, const std::string& ending) {
    return boost::ends_with(original, ending);
}

std::vector<std::string> string_utilities::split(const std::string& original, const std::string& delims, bool compress) {
    std::vector<std::string> elements;
    if (original.empty()) return elements;
    auto token_compress = compress ? boost::token_compress_on : boost::token_compress_off;
    boost::split(elements, original, boost::is_any_of(delims), token_compress);
    return elements;
}

std::string string_utilities::join(const std::vector<std::string>& strings, const std::string& separator) {
    return boost::join(strings, separator);
}

bool string_utilities::string_to_bool(const std::string& original) {
    auto capitalized = capitalize(original);
    if (capitalized == "TRUE" || capitalized == "1")
        return true;
    if (capitalized != "FALSE" && capitalized != "0")
        LOGGER_W << "Unable to convert string " << original << " to boolean. Defaulting to false.";
    return false;
}

void string_utilities::replace_all(std::string& haystack, const std::string& needle, const std::string& replacement) {
    boost::replace_all(haystack, needle, replacement);
}

void string_utilities::normalize_slashes(std::string& filename) {
    replace_all(filename, "\\", "/");
}
