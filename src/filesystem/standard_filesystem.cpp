#ifndef OCB_USE_BOOST_FILESYSTEM
#include "../../include/filesystem/standard_filesystem.hpp"
#include "../../include/log.hpp"
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

bool Standard_Filesystem::exists(const std::string& path) {
    return fs::exists(fs::u8path(path));
}

bool Standard_Filesystem::is_regular_file(const std::string& path) {
    try {
        return fs::is_regular_file(fs::u8path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error while trying to check if path is a regular file: "
            << path << ", assuming it isn't - " << e.what();
    }
    return false;
}

bool Standard_Filesystem::is_directory(const std::string& path) {
    try {
        return fs::is_directory(fs::u8path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error while trying to check if path is a directory: " << path << ", assuming it isn't - " << e.what();
    }
    return false;
}

std::tuple<unsigned long long, std::tm> Standard_Filesystem::last_write_time(const std::string& path) {
    try {
        auto file_time = fs::last_write_time(path);
        auto duration = file_time - fs::file_time_type::clock::now()
            + std::chrono::system_clock::now();
        auto time_point = std::chrono::time_point_cast<std::chrono::system_clock::duration>(duration);
        auto time_t = std::chrono::system_clock::to_time_t(time_point);

        auto since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count();
#pragma warning(push)
#pragma warning(disable: 4996)
        return std::make_tuple(static_cast<unsigned long long>(since_epoch), *std::localtime(&time_t));
#pragma warning(pop)
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error getting the last modified time for path: " << path << ", returning 0s - " << e.what();
    }

    return std::make_tuple(0ULL, std::tm{});
}

std::vector<std::string> Standard_Filesystem::directory_content_names_unchecked(const std::string& path) {
    std::vector<std::string> result;

    for (auto& p : fs::directory_iterator(fs::u8path(path))) {
        if (fs::is_regular_file(p) || fs::is_directory(p)) {
            result.emplace_back(p.path().filename().string());
        }
    }

    return result;
}

std::vector<Path_Info> Standard_Filesystem::directory_content_details_unchecked(const std::string& path) {
    std::vector<Path_Info> result;

    for (auto& p : fs::directory_iterator(fs::u8path(path))) {
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

bool Standard_Filesystem::is_absolute_path(const std::string& path) {
    return fs::path{ fs::u8path(path) }.is_absolute();
}

std::string Standard_Filesystem::get_filename_component(const std::string& path) {
    return fs::path{ fs::u8path(path) }.filename().string();
}

std::string Standard_Filesystem::get_stem_component(const std::string& path) {
    return fs::path{ fs::u8path(path) }.stem().string();
}

bool Standard_Filesystem::copy(const std::string& source, const std::string& destination) {
    try {
        fs::copy(fs::u8path(source), fs::u8path(destination),
            fs::copy_options::overwrite_existing | fs::copy_options::recursive);

        return true;
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error copying " << source << " to " << destination << ": " << e.what();
        return false;
    }
}

bool Standard_Filesystem::remove(const std::string& path) {
    try {
        if (is_directory(path))
            return fs::remove_all(fs::u8path(path));
        else
            return fs::remove(fs::u8path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error deleting file " << path << ": " << e.what();
        return false;
    }
}

bool Standard_Filesystem::create_directories(const std::string& path) {
    auto fs_path = fs::u8path(path);
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
