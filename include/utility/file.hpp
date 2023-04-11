#ifndef HPP_UTILITY_FILE
#define HPP_UTILITY_FILE

#include <string_view>
#include <memory>
#include "filesystem/user_data_folder.hpp"

namespace file_utilities {
    // Get a non-virtual filesystem implementation such as std or boost
    std::shared_ptr<Writable_Filesystem> disk_filesystem(std::string_view arg = {});
    // Get a virtual filesystem implementation such as PhysFS
    std::shared_ptr<Readable_Filesystem> virtual_filesystem(std::string_view arg = {}, std::string_view archive_name = "");
    // Get the filesystem used to store game data files
    Readable_Filesystem* game_data_filesystem(std::string_view arg = {});
    // Override the game data filesystem
    void set_game_data_filesystem(Readable_Filesystem* filesystem);
    // Get the filesystem used to read the default (non-user) config file. Can be
    // different from game data in the case of executable + config.ini + game archive
    Readable_Filesystem* default_config_filesystem(std::string_view arg = {});
    // Override the default config file filesystem
    void set_default_config_filesystem(Readable_Filesystem* filesystem);
    // Get the filesystem used to read/write user data (config, log, save files)
    Writable_Filesystem* user_data_filesystem(std::string_view arg = {});
    // Override the user data filesystem
    void set_user_data_filesystem(Writable_Filesystem* filesystem);
    // Get a helper class for accessing and writing to the user data folder
    std::shared_ptr<User_Data_Folder> user_data_folder();
    // Try to open a URL
    bool open_url(const std::string& url);
}

#endif