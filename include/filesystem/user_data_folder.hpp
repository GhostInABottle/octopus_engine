#ifndef HPP_USER_DATA_FOLDER
#define HPP_USER_DATA_FOLDER

#include "../log_levels.hpp"
#include "writable_filesystem.hpp"
#include <string>
#include <tuple>
#include <vector>

class Save_File;
class Key_Binder;
class Environment;

// The folder where user data (save files, config, log, etc.) are written
class User_Data_Folder {
public:
    // The constructor will get the system's data path and create the folder if needed
    User_Data_Folder(Writable_Filesystem& filesystem, const Environment& env);
    // Destructor
    ~User_Data_Folder() {}
    // Parse config file and save it to data directory if needed
    void parse_config();
    // Parse the default config (not the one in user data directory)
    static std::vector<std::tuple<Log_Level, std::string>> parse_default_config();
    // Save config file if it changed since last save (or if forced)
    void save_config(bool force = false, bool write_logs = true);
    // Try to save the file, logging any errors
    bool try_to_save_config();
    // Save the Lua data in the save file to the specified filename
    bool save(std::string filename, Save_File& save_file);
    // Load the saved data at file name into the save file
    bool load(std::string filename, Save_File& save_file);
    // Get the filename of the keymap file
    std::string get_keymap_filename() const;
    // Load key binding file
    bool load_keymap_file(Key_Binder& key_binder);
    // Save key binding file
    bool save_keymap_file(Key_Binder& key_binder);
    // Get the base path for user data (e.g. C:\users\abc\Documents\My Games\)
    std::string get_base_path() const { return base_path; }
    // Get the game specific folder (e.g. C:\users\abc\Documents\My Games\GameTitle\)
    std::string get_game_path() const { return game_path; }
    // In certain scenarios (e.g. on Steam), get user's folder, otherwise it's the same as game path
    // (e.g. C:\users\abc\Documents\My Games\GameTitle\123\)
    std::string get_user_path() const { return user_path; }
    // Get the folder for the current version (e.g. C:\users\abc\Documents\My Games\GameTitle\123\v2\)
    std::string get_version_path() const { return version_path; }
    // Get the underlying filesystem
    Writable_Filesystem& get_filesystem() { return filesystem; }
    // Get any warnings or logs encountered when getting the system path
    const std::vector<std::tuple<Log_Level, std::string>>& get_logs() const { return logs; }
    // Clear the logs
    void clear_logs() { logs.clear(); }
private:
    Writable_Filesystem& filesystem;
    std::string base_path;
    std::string game_path;
    std::string user_path;
    std::string version_path;
    static bool parsed_default_config;
    std::vector<std::tuple<Log_Level, std::string>> logs;
    // Add directory to save filename
    void cleanup_save_filename(std::string& filename) const;
    // Copy an older version of the data folder to the latest one
    void try_to_copy_old_version();
};

#endif
