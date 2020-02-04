#include "../../include/utility/string.hpp"
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <sstream>

void trim(std::string& s) {
    boost::trim(s);
}

void capitalize(std::string& original) {
    boost::to_upper(original);
}

bool equal_strings(const std::string& str1, const std::string& str2) {
    return boost::iequals(str1, str2);
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