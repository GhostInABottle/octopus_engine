#ifndef HPP_UTILITY_FILE
#define HPP_UTILITY_FILE

#include <memory>
#include "filesystem/user_data_folder.hpp"

namespace file_utilities {
    std::shared_ptr<Writable_Filesystem> disk_filesystem();
    Readable_Filesystem* game_data_filesystem(const char* arg = nullptr);
    void set_game_data_filesystem(Readable_Filesystem* filesystem);
    Writable_Filesystem* user_data_filesystem(const char* arg = nullptr);
    void set_user_data_filesystem(Writable_Filesystem* filesystem);
    std::shared_ptr<User_Data_Folder> user_data_folder();
}

#endif