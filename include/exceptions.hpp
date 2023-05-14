#ifndef HPP_EXCEPTIONS
#define HPP_EXCEPTIONS

#include <stdexcept>
#include <string>
#include <system_error>
#include <cerrno>

// XML file loading exception
class xml_exception : public std::runtime_error {
public:
    explicit xml_exception(const std::string& error) : std::runtime_error(error) {}
};

// TMX map format loading exception
class tmx_exception : public xml_exception {
public:
    explicit tmx_exception(const std::string& error) : xml_exception(error) {}
};

// Errors when opening files
class file_loading_exception : public std::system_error {
public:
    explicit file_loading_exception(const std::string& error)
        : std::system_error(errno, std::generic_category(), error) {}
};

#endif
