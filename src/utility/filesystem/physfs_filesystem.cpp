#include "../../../include/utility/filesystem/physfs_filesystem.hpp"
#include "../../../include/vendor/physfs.hpp"
#include "../../../include/utility/string.hpp"

std::unique_ptr<std::istream> PhysFS_Filesystem::open_ifstream(std::string filename, std::ios_base::openmode) {
    return open_binary_ifstream(filename);
}

std::unique_ptr<std::istream> PhysFS_Filesystem::open_binary_ifstream(std::string filename) {
    string_utilities::normalize_slashes(filename);
    return std::make_unique<PhysFS::ifstream>(filename);
}

std::string PhysFS_Filesystem::read_file(std::string filename) {
    return "";
}

bool PhysFS_Filesystem::file_exists(const std::string& filename) {
    return false;
}

bool PhysFS_Filesystem::is_regular_file(const std::string& path) {
    return false;
}

bool PhysFS_Filesystem::is_directory(const std::string& path) {
    return false;
}

std::tuple<unsigned long long, std::tm> PhysFS_Filesystem::last_write_time(const std::string& path) {
    return std::make_tuple(0ull, std::tm{});
}

bool PhysFS_Filesystem::is_absolute_path(const std::string& path) {
    return false;
}

std::string PhysFS_Filesystem::get_filename_component(const std::string& path) {
    return path;
}

std::string PhysFS_Filesystem::get_stem_component(const std::string& path) {
    return path;
}

std::vector<std::string> PhysFS_Filesystem::directory_content_names(const std::string& path) {
    return {};
}

std::vector<Path_Info> PhysFS_Filesystem::directory_content_details(const std::string& path) {
    return {};
}