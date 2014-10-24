#ifndef HPP_EXCEPTIONS
#define HPP_EXCEPTIONS

#include <stdexcept>

// XML file loading exception
class xml_exception : public std::runtime_error {
public:
    xml_exception(const char* error) : std::runtime_error(error) {}
};

// TMX map format loading exception
class tmx_exception : public xml_exception {
public:
    tmx_exception(const char* error) : xml_exception(error) {}
};

#endif
