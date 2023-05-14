#include "../../../include/filesystem/physfs_filesystem.hpp"
#include "../../../include/vendor/physfs.hpp"
#include "../../../include/utility/string.hpp"
#include "../../../include/log.hpp"
#include "../../../include/exceptions.hpp"
#include <chrono>

namespace detail {
    static std::string clean_relative_path(const std::string& path) {
        // PhysFS doesn't like . in relative paths
        auto starts_with_dot = string_utilities::starts_with(path, ".");
        auto cleaned = starts_with_dot ? path.substr(1) : path;
        string_utilities::normalize_slashes(cleaned);
        return cleaned;
    }
}

PhysFS_Filesystem::PhysFS_Filesystem(std::string_view arg, std::string_view archive_name) {
    PhysFS::init(std::string{ arg }.c_str());

    auto base_dir = PhysFS::getBaseDir();
    string_utilities::normalize_slashes(base_dir);

    if (string_utilities::ends_with(base_dir, "Debug/")
            || string_utilities::ends_with(base_dir, "Release/")) {
        // Hack for running in Visual Studio
        base_dir = base_dir.substr(0, base_dir.find_last_of("/"));
        base_dir = base_dir.substr(0, base_dir.find_last_of("/") + 1);
    }

    if (!archive_name.empty()) {
        PhysFS::mount(base_dir + std::string{ archive_name }, "", true);
    }

    PhysFS::mount(base_dir, "", true);
}

PhysFS_Filesystem::~PhysFS_Filesystem() {
    PhysFS::deinit();
}

std::unique_ptr<std::istream> PhysFS_Filesystem::open_ifstream(std::string filename, std::ios_base::openmode) {
    return open_binary_ifstream(filename);
}

std::unique_ptr<std::istream> PhysFS_Filesystem::open_binary_ifstream(std::string filename) {
    return std::make_unique<PhysFS::ifstream>(detail::clean_relative_path(filename));
}

std::string PhysFS_Filesystem::read_file(std::string filename) {
    auto stream = open_binary_ifstream(filename);
    if (!stream || !*stream) {
        throw file_loading_exception("Couldn't open file for reading: " + filename);
    }

    return std::string((std::istreambuf_iterator<char>(*stream)),
        std::istreambuf_iterator<char>());
}

bool PhysFS_Filesystem::file_exists(const std::string& filename) {
    return PhysFS::exists(detail::clean_relative_path(filename));
}

bool PhysFS_Filesystem::is_regular_file(const std::string& path) {
    auto clean = detail::clean_relative_path(path);
    return !PhysFS::isDirectory(clean) && !PhysFS::isSymbolicLink(clean);
}

bool PhysFS_Filesystem::is_directory(const std::string& path) {
    return PhysFS::isDirectory(detail::clean_relative_path(path));
}

std::tuple<unsigned long long, std::tm> PhysFS_Filesystem::last_write_time(const std::string& path) {
    auto mod_time = PhysFS::getLastModTime(detail::clean_relative_path(path));
    if (mod_time == -1) return std::make_tuple(0ull, std::tm{});

    auto time_t = static_cast<std::time_t>(mod_time);
    auto time_point = std::chrono::system_clock::from_time_t(time_t);

    auto since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count();
#pragma warning(push)
#pragma warning(disable: 4996)
    auto result = std::make_tuple(static_cast<unsigned long long>(since_epoch), *std::localtime(&time_t));
#pragma warning(pop)

    return result;
}

bool PhysFS_Filesystem::is_absolute_path(const std::string& path) {
    return false;
}

std::string PhysFS_Filesystem::get_filename_component(const std::string& path) {
    std::string copy{path};
    string_utilities::normalize_slashes(copy);
    auto last_slash = copy.find_last_of('/');
    if (last_slash == std::string::npos) return copy;

    return copy.substr(last_slash + 1);
}

std::string PhysFS_Filesystem::get_stem_component(const std::string& path) {
    auto filename_component = get_filename_component(path);
    auto last_dot = filename_component.find_last_of('.');
    if (last_dot == std::string::npos) return filename_component;

    return filename_component.substr(0, last_dot);
}

std::vector<std::string> PhysFS_Filesystem::directory_content_names(const std::string& path) {
    if (!is_directory(path)) {
        LOGGER_E << "Tried to list a path that is not a directory " << path;
        return {};
    }

   return PhysFS::enumerateFiles(detail::clean_relative_path(path));
}

std::vector<Path_Info> PhysFS_Filesystem::directory_content_details(const std::string& path) {
    if (!is_directory(path)) {
        LOGGER_E << "Tried to list a path that is not a directory " << path;
        return {};
    }
    auto dir_name = detail::clean_relative_path(path);
    if (!string_utilities::ends_with(dir_name, "/")) {
        dir_name += "/";
    }

    std::vector<Path_Info> result;
    auto filenames = PhysFS::enumerateFiles(dir_name);
    for (auto& filename : filenames) {
        auto full_name = dir_name + filename;
        auto is_regular = is_regular_file(full_name);
        auto is_dir = is_directory(full_name);
        if (!is_regular && !is_dir) continue;

        Path_Info path_info;
        path_info.name = filename;
        path_info.is_regular = is_regular;
        path_info.is_directory = is_dir;
        auto [timestamp, tm] = last_write_time(full_name);
        path_info.timestamp = timestamp;
        path_info.calendar_time = tm;

        result.push_back(path_info);
    }

    return result;
}