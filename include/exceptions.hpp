#ifndef HPP_EXCEPTIONS
#define HPP_EXCEPTIONS

#include <stdexcept>
#include <string>

// XML file loading exception
class xml_exception : public std::runtime_error {
public:
    explicit xml_exception(const std::string& error) : std::runtime_error(error) {}
    explicit xml_exception(const char* error) : std::runtime_error(error) {}
};

// TMX map format loading exception
class tmx_exception : public xml_exception {
public:
    explicit tmx_exception(const std::string& error) : xml_exception(error) {}
    explicit tmx_exception(const char* error) : xml_exception(error) {}
};

#endif
