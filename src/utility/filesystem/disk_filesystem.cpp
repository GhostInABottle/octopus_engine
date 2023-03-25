#include "../../../include/utility/filesystem/disk_filesystem.hpp"
#include "../../../include/utility/string.hpp"
#include "../../../include/log.hpp"
#ifdef _WIN32
#include "../../../include/vendor/utf8conv.h"
#endif
#include <stdexcept>
#include <fstream>

std::unique_ptr<std::istream> Disk_Filesystem::open_ifstream(std::string filename, std::ios_base::openmode mode) {
    string_utilities::normalize_slashes(filename);

#ifdef _WIN32
    auto utf16_filename = win32::Utf8ToUtf16(filename);
    return std::make_unique<std::ifstream>(utf16_filename, mode);
#else
    return std::make_unique<std::ifstream>(filename, mode);
#endif
}

std::unique_ptr<std::istream> Disk_Filesystem::open_binary_ifstream(std::string filename) {
    return open_ifstream(filename, std::ios_base::in | std::ios_base::binary);
}

std::unique_ptr<std::ostream> Disk_Filesystem::open_ofstream(std::string filename, std::ios_base::openmode mode) {
    string_utilities::normalize_slashes(filename);

#ifdef _WIN32
    auto utf16_filename = win32::Utf8ToUtf16(filename);
    return std::make_unique<std::ofstream>(utf16_filename , mode);
#else
    return std::make_unique<std::ofstream>(filename, mode);
#endif
}

std::string Disk_Filesystem::read_file(std::string filename) {
    auto stream = open_ifstream(filename);
    if (!stream || !*stream) {
        throw std::runtime_error("Couldn't open file for reading: " + filename);
    }

    return std::string((std::istreambuf_iterator<char>(*stream)),
        std::istreambuf_iterator<char>());
}

std::vector<std::string> Disk_Filesystem::directory_content_names(const std::string& path) {
    if (!is_directory(path)) {
        LOGGER_E << "Tried to list a path that is not a directory " << path;
        return {};
    }

    try {
        return directory_content_names_unchecked(path);
    } catch (std::runtime_error& e) {
        LOGGER_E << "Error while trying to list directory " << path << ", some or all files might be ignored - " << e.what();
        return {};
    }
}

std::vector<Path_Info> Disk_Filesystem::directory_content_details(const std::string& path) {
    if (!is_directory(path)) {
        LOGGER_E << "Tried to list a path that is not a directory " << path;
        return {};
    }

    std::string dir_name{path};
    string_utilities::normalize_slashes(dir_name);
    if (!string_utilities::ends_with(dir_name, "/")) {
        dir_name += "/";
    }

    try {
        return directory_content_details_unchecked(dir_name);
    } catch (std::runtime_error& e) {
        LOGGER_E << "Error while trying to list directory " << path << ", some or all files might be ignored - " << e.what();
        return {};
    }
}