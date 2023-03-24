#include "../../include/utility/file.hpp"
#include "../../include/utility/filesystem/standard_filesystem.hpp"
#include "../../include/utility/filesystem/boost_filesystem.hpp"
#include "../../include/utility/filesystem/physfs_filesystem.hpp"

namespace file_utilities::detail {
    std::shared_ptr<Writable_Filesystem> disk_filesystem;
    std::shared_ptr<Readable_Filesystem> virtual_filesystem;
    Writable_Filesystem* user_data_filesystem{ nullptr };
    Readable_Filesystem* game_data_filesystem{ nullptr };
    std::shared_ptr<User_Data_Folder> user_data_folder;
}

std::shared_ptr<Writable_Filesystem> file_utilities::disk_filesystem(const char*) {
    if (detail::disk_filesystem) return detail::disk_filesystem;

#ifdef OCB_USE_BOOST_FILESYSTEM
    detail::disk_filesystem = std::make_shared<Boost_Filesystem>();
#else
    detail::disk_filesystem = std::make_shared<Standard_Filesystem>();
#endif

    return detail::disk_filesystem;
}

std::shared_ptr<Readable_Filesystem> file_utilities::virtual_filesystem(const char* arg) {
    if (detail::virtual_filesystem) return detail::virtual_filesystem;

    detail::virtual_filesystem = std::make_shared<PhysFS_Filesystem>(arg);
    return detail::virtual_filesystem;
}

Readable_Filesystem* file_utilities::game_data_filesystem(const char* arg) {
    if (detail::game_data_filesystem) return detail::game_data_filesystem;

    auto disk_fs = virtual_filesystem(arg);
    detail::game_data_filesystem = disk_fs.get();
    return detail::game_data_filesystem;
}

void file_utilities::set_game_data_filesystem(Readable_Filesystem* fs) {
    detail::game_data_filesystem = fs;
}

Writable_Filesystem* file_utilities::user_data_filesystem(const char* arg) {
    if (detail::user_data_filesystem) return detail::user_data_filesystem;

    auto disk_fs = disk_filesystem();
    detail::user_data_filesystem = disk_fs.get();
    return detail::user_data_filesystem;
}

void file_utilities::set_user_data_filesystem(Writable_Filesystem* fs) {
    detail::user_data_filesystem = fs;
}

std::shared_ptr<User_Data_Folder> file_utilities::user_data_folder() {
    if (detail::user_data_folder) return detail::user_data_folder;

    auto fs = user_data_filesystem();
    detail::user_data_folder = std::make_shared<User_Data_Folder>(*fs);
    return detail::user_data_folder;
}
