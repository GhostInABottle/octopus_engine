#ifdef OCB_USE_BOOST_FILESYSTEM
#include "../../include/filesystem/boost_filesystem.hpp"
#include "../../include/log.hpp"
#ifdef _WIN32
#include "../../include/vendor/utf8conv.h"
#endif
#include <boost/filesystem.hpp>
#include <chrono>

namespace fs = boost::filesystem;

namespace detail {
    static fs::path string_to_utf8_path(const std::string& str) {
        #ifdef _WIN32
                return fs::path(win32::Utf8ToUtf16(str));
        #else
                return fs::path(str);
        #endif
    }
}

bool Boost_Filesystem::file_exists(const std::string& filename) {
    return fs::exists(detail::string_to_utf8_path(filename));
}

bool Boost_Filesystem::is_regular_file(const std::string& path) {
    try {
        return fs::is_regular_file(detail::string_to_utf8_path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error while trying to check if path is a regular file: "
            << path << ", assuming it isn't - " << e.what();
    }
    return false;
}

bool Boost_Filesystem::is_directory(const std::string& path) {
    try {
        return fs::is_directory(detail::string_to_utf8_path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error while trying to check if path is a directory: " << path << ", assuming it isn't - " << e.what();
    }
    return false;
}

std::tuple<unsigned long long, std::tm> Boost_Filesystem::last_write_time(const std::string& path) {
    try {
        auto file_time = fs::last_write_time(path);
        auto time_point = std::chrono::system_clock::from_time_t(file_time);

        auto since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count();
#pragma warning(push)
#pragma warning(disable: 4996)
        return std::make_tuple(static_cast<unsigned long long>(since_epoch), *std::localtime(&file_time));
#pragma warning(pop)
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error getting the last modified time for path: " << path << ", returning 0s - " << e.what();
    }

    return std::make_tuple(0ULL, std::tm{});
}

std::vector<std::string> Boost_Filesystem::directory_content_names_unchecked(const std::string& path) {
    std::vector<std::string> result;

    for (auto& p : fs::directory_iterator(detail::string_to_utf8_path(path))) {
        if (fs::is_regular_file(p) || fs::is_directory(p)) {
            result.emplace_back(p.path().filename().string());
        }
    }

    return result;
}

std::vector<Path_Info> Boost_Filesystem::directory_content_details_unchecked(const std::string& path) {
    std::vector<Path_Info> result;

    for (auto& p : fs::directory_iterator(detail::string_to_utf8_path(path))) {
        auto is_regular = fs::is_regular_file(p);
        auto is_directory = fs::is_directory(p);
        if (!is_regular && !is_directory) continue;

        Path_Info path_info;
        path_info.name = p.path().filename().string();
        path_info.is_regular = is_regular;
        path_info.is_directory = is_directory;
        auto [timestamp, tm] = last_write_time(path + path_info.name);
        path_info.timestamp = timestamp;
        path_info.calendar_time = tm;

        result.push_back(path_info);
    }

    return result;
}

bool Boost_Filesystem::is_absolute_path(const std::string& path) {
    return fs::path{ detail::string_to_utf8_path(path) }.is_absolute();
}

std::string Boost_Filesystem::get_filename_component(const std::string& path) {
    return fs::path{ detail::string_to_utf8_path(path) }.filename().string();
}

std::string Boost_Filesystem::get_stem_component(const std::string& path) {
    return fs::path{ detail::string_to_utf8_path(path) }.stem().string();
}

bool Boost_Filesystem::copy_file(const std::string& source, const std::string& destination) {
    try {
        fs::copy_file(detail::string_to_utf8_path(source), detail::string_to_utf8_path(destination),
            fs::copy_option::overwrite_if_exists);

        return true;
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error copying " << source << " to " << destination << ": " << e.what();
        return false;
    }
}

bool Boost_Filesystem::remove_file(const std::string& filename) {
    try {
        return fs::remove(detail::string_to_utf8_path(filename));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error deleting file " << filename << ": " << e.what();
        return false;
    }
}

bool Boost_Filesystem::create_directories(const std::string& path) {
    if (file_exists(path)) return true;
    try {
        return fs::create_directories(detail::string_to_utf8_path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error creating directories " << path << ": " << e.what();
        return false;
    }
}
#endif