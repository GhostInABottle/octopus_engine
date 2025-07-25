#ifdef OCB_USE_BOOST_FILESYSTEM
#include "boost_filesystem.hpp"
#include "../log.hpp"
#ifdef _WIN32
#include "../vendor/utf8conv.h"
#endif
#include <boost/version.hpp>
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

bool Boost_Filesystem::exists(const std::string& path) {
    return fs::exists(detail::string_to_utf8_path(path));
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
        auto file_time = fs::last_write_time(detail::string_to_utf8_path(path));
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

std::uintmax_t Boost_Filesystem::file_size(const std::string& path) {
    try {
        auto fs_path = detail::string_to_utf8_path(path);
        if (!fs::exists(fs_path) || !fs::is_regular_file(fs_path)) return 0;

        return fs::file_size(fs_path);
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error getting the file size of: " << path << ", returning 0 - " << e.what();
    }

    return 0;
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

std::string Boost_Filesystem::filename_component(const std::string& path) {
    return fs::path{ detail::string_to_utf8_path(path) }.filename().string();
}

std::string Boost_Filesystem::stem_component(const std::string& path) {
    return fs::path{ detail::string_to_utf8_path(path) }.stem().string();
}

std::string Boost_Filesystem::extension(const std::string& path) {
    return fs::path{ detail::string_to_utf8_path(path) }.extension().string();
}

bool Boost_Filesystem::copy(const std::string& source, const std::string& destination) {
    try {
#if BOOST_VERSION >= 107400
        fs::copy(detail::string_to_utf8_path(source),
            detail::string_to_utf8_path(destination),
            fs::copy_options::overwrite_existing | fs::copy_options::recursive);

        return true;
#else
        // Older Boost FS didn't copy directories and used different options
        auto source_is_dir = is_directory(source);
        auto destination_exists = exists(destination);
        auto destination_path = detail::string_to_utf8_path(destination);
        if (source_is_dir) {
            if (destination_exists) {
                if (!remove(destination)) return false;
            }
            auto created = create_directories(destination);
            if (!created) return false;

            for (auto& entry : fs::directory_iterator(source)) {
                auto copied = copy(entry.path().string(),
                    (destination_path / entry.path().filename()).string());
                if (!copied) {
                    remove(destination);
                    return false;
                }
            }

        } else {
            fs::copy_file(detail::string_to_utf8_path(source), destination_path,
                fs::copy_option::overwrite_if_exists);
        }

        return true;
#endif
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error copying " << source << " to " << destination << ": " << e.what();
        return false;
    }
}

bool Boost_Filesystem::remove(const std::string& path) {
    try {
        if (is_directory(path))
            return fs::remove_all(detail::string_to_utf8_path(path));
        else
            return fs::remove(detail::string_to_utf8_path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error deleting file " << path << ": " << e.what();
        return false;
    }
}

bool Boost_Filesystem::rename(const std::string& old_path, const std::string& new_path) {
    try {
        fs::rename(detail::string_to_utf8_path(old_path), detail::string_to_utf8_path(new_path));
        return true;
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error renameing file " << old_path << " to " << new_path << ": " << e.what();
        return false;
    }
}

bool Boost_Filesystem::create_directories(const std::string& path) {
    auto fs_path = detail::string_to_utf8_path(path);
    if (fs::exists(fs_path)) return true;

    try {
        fs::create_directories(fs_path);
        // Can't rely on create_directories's return value; it returns false for trailing /
        return fs::exists(fs_path);
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error creating directory " << path << ": " << e.what();
        return false;
    }
}

#endif
