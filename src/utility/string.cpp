#include "../../include/utility/string.hpp"
#include <boost/algorithm/string.hpp>
#include <sstream>

void trim(std::string& s) {
    boost::trim(s);
}

std::string trim(const std::string& s) {
    std::string copy{s};
    trim(copy);
    return copy;
}

void capitalize(std::string& original) {
    boost::to_upper(original);
}

std::string capitalize(const std::string& original) {
    std::string copy{original};
    capitalize(copy);
    return copy;
}

bool equal_strings(const std::string& str1, const std::string& str2) {
    return boost::iequals(str1, str2);
}

std::vector<std::string> split(const std::string& original, const std::string& delims, bool compress) {
    std::vector<std::string> elements;
    if (original.empty()) return elements;
    auto token_compress = compress ? boost::token_compress_on : boost::token_compress_off;
    boost::split(elements, original, boost::is_any_of(delims), token_compress);
    return elements;
}

std::string join(const std::vector<std::string>& strings, const std::string& separator) {
    return boost::join(strings, separator);
}