#include "../../include/configurations.hpp"
#include "../../include/environments/environment.hpp"
#include "../../include/exceptions.hpp"
#include "../../include/filesystem/user_data_folder.hpp"
#include "../../include/key_binder.hpp"
#include "../../include/log.hpp"
#include "../../include/save_file.hpp"
#include "../../include/utility/file.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/vendor/platform_folders.hpp"
#include <stdexcept>

bool User_Data_Folder::parsed_default_config = false;

void User_Data_Folder::try_to_copy_old_version() {
    std::string version_to_copy;
    auto copy_old_version = Configurations::get<std::string>("game.copy-old-data-folder");
    if (copy_old_version.empty()) return;

    auto versions = string_utilities::split(copy_old_version, ",");
    for (const auto& version : versions) {
        auto trimmed_version = string_utilities::trim(version);
        if (trimmed_version.empty()) continue;

        auto old_version_path = user_path + trimmed_version + "/";
        if (filesystem.exists(old_version_path)) {
            version_to_copy = old_version_path;
            break;
        }
    }

    // Only copy if we found a valid old version path
    auto has_old_version = !version_to_copy.empty() && version_to_copy != version_path;
    if (!has_old_version) return;

    // If the new version folder doesn't exist, copy the old one
    auto folder_exists = filesystem.exists(version_path);
    if (!folder_exists) {
        logs.push_back(std::make_tuple(Log_Level::info,
            "Copying data folder from old version " + version_to_copy
            + " to " + version_path));
        filesystem.copy(version_to_copy, version_path);
        return;
    }

    // If there is a new version folder, and it has a config file, do nothing
    if (filesystem.exists(version_path + "config.ini")) {
        logs.push_back(std::make_tuple(Log_Level::info,
            "Data folder exists. Not copying old version data " + version_to_copy));
        return;
    }

    // If the new version folder doesn't have a config file, try to copy the old one
    // This could happen if cloud saves are copied (without config) before starting
    auto old_config_path = version_to_copy + "config.ini";
    logs.push_back(std::make_tuple(Log_Level::info,
        "Copying config files from old version " + old_config_path
        + " to " + version_path));

    if (filesystem.exists(old_config_path)) {
        filesystem.copy(old_config_path, version_path + "config.ini");
    } else {
        logs.push_back(std::make_tuple(Log_Level::warning,
            "Config file was not found in old folder"));
    }

    // Do the same for the keymap file
    std::string keymap_filename = Configurations::get<std::string>("controls.mapping-file");
    auto keymap_path_absolute = filesystem.is_absolute_path(keymap_filename);
    if (keymap_path_absolute || filesystem.exists(version_path + keymap_filename)) return;

    auto old_keymap_path = version_to_copy + keymap_filename;
    // No warning if it doesn't exist—it just means the mappings weren't modified
    if (!filesystem.exists(old_keymap_path)) return;

    logs.push_back(std::make_tuple(Log_Level::info,
        "Copying keymap file from old version " + old_keymap_path
        + " to " + version_path));
    filesystem.copy(old_keymap_path, version_path + keymap_filename);
}

User_Data_Folder::User_Data_Folder(Writable_Filesystem& filesystem, const Environment& env)
        : filesystem(filesystem) {
    std::string default_folder;
    auto add_game_folder = false;

    if (!Configurations::defaults_loaded()) {
        parse_config();
    }

    auto config_folder = Configurations::get<std::string>("game.data-folder");
    string_utilities::trim(config_folder);

    if (config_folder.empty()) {
        try {
            default_folder = sago::getDataHome();
            add_game_folder = true;
        } catch (std::runtime_error& e) {
            logs.push_back(std::make_tuple(Log_Level::warning,
                "Unable to retrieve default save folder. Using executable folder - "
                    + std::string{ e.what() }));
        }
    } else {
        default_folder = config_folder;
        if (default_folder == ".") {
            default_folder = "";
        } else if (default_folder.rfind("./", 0) == 0) {
            // Remove ./
            default_folder = default_folder.substr(2);
        }
    }

    if (default_folder.empty()) {
        return;
    }

    string_utilities::normalize_slashes(default_folder);
    if (default_folder.back() != '/') {
        default_folder += '/';
    }

    // We break up the path as follows: base_path/game_path/user_path/version_path
    // The base path is the home directory for data files e.g. ~/AppData/Roaming
    base_path = default_folder;

    if (add_game_folder) {
        // Game path is the game-specific folder. E.g. ~/AppData/Roaming/GameTitle
        auto title = Configurations::get<std::string>("game.title");
        string_utilities::trim(title);
        if (title.empty()) {
            title = "OctopusEngine";
        }
        default_folder += title + "/";
        game_path = default_folder;

        // In some environments (e.g. Steam), each user account has its own folder
        // The user path will include the user ID. E.g. ~/AppData/Roaming/GameTitle/123
        // If the environment doesn't have user IDs, it'll be the same as the game path
        auto user_id = env.get_user_id_string();
        if (!user_id.empty()) {
            default_folder += user_id + "/";
        }
        user_path = default_folder;

        // Finally, there will be a folder for each "version" of user data,
        // as specified in the config file. The version path is where config
        // and save files are written. E.g. ~/AppData/Roaming/Gametitle/123/V5
        auto data_dir_version = Configurations::get<std::string>("game.data-folder-version");
        string_utilities::trim(data_dir_version);
        default_folder += data_dir_version + "/";
    } else {
        game_path = default_folder;
        user_path = default_folder;
    }

    version_path = default_folder;

    // Check if we can copy old versions of the folder
    try_to_copy_old_version();

    if (filesystem.exists(default_folder)) return;

    // Create the folder if needed
    logs.push_back(std::make_tuple(Log_Level::info,
        "Creating user data folder: " + default_folder));
    auto created = filesystem.create_directories(default_folder);
    if (created) return;

    logs.push_back(std::make_tuple(Log_Level::warning,
        "Unable to create save folder. " + default_folder
        + " Using the executable's folder instead"));

    base_path = "";
    game_path = "";
    user_path = "";
    version_path = "";
}

void User_Data_Folder::parse_config() {
    if (!parsed_default_config) {
        throw config_exception{"Trying to parse user config but default configs"
            " were not loaded. Make sure parse_default_config is called"};
    }

    if (version_path.empty()) return;

    auto config_path = version_path + "config.ini";
    try {
        if (!filesystem.exists(config_path)) {
            save_config(true, false);
            return;
        }

        auto existing_file_stream = filesystem.open_ifstream(config_path);
        if (!existing_file_stream || !*existing_file_stream) {
            throw config_exception("Couldn't read file " + config_path);
        }

        auto new_warnings = Configurations::parse(*existing_file_stream, false);

        logs.push_back(std::make_tuple(Log_Level::info, "Parsed " + config_path));

        for (auto& warning : new_warnings) {
            logs.push_back(std::make_tuple(Log_Level::warning, warning));
        }
    } catch (config_exception& e) {
        logs.push_back(std::make_tuple(Log_Level::error,
            "Unable to write config file to " + version_path + ": " + e.what()));
    }
}

std::vector<std::tuple<Log_Level, std::string>> User_Data_Folder::parse_default_config() {
    if (parsed_default_config) return {};

    auto config_fs = file_utilities::default_config_filesystem();
    auto default_stream = config_fs->open_ifstream("config.ini");
    if (!default_stream || !*default_stream) {
        throw config_exception("Couldn't read file config.ini");
    }

    auto warnings = Configurations::parse(*default_stream, true);

    std::vector<std::tuple<Log_Level, std::string>> logs;
    logs.push_back(std::make_tuple(Log_Level::info, "Parsed the default config.ini"));

    for (auto& warning : warnings) {
        logs.push_back(std::make_tuple(Log_Level::warning, warning));
    }

    parsed_default_config = true;

    return logs;
}

void User_Data_Folder::save_config(bool force, bool write_logs) {
    auto update_config = Configurations::get<bool>("debug.update-config-files");
    auto save = force || (update_config && Configurations::changed());
    if (!save) {
        return;
    }

    auto config_path = version_path + "config.ini";

    auto stream = filesystem.open_ofstream(config_path);
    if (!stream || !*stream) {
        throw file_loading_exception("Couldn't open config file for saving: " + config_path);
    }

    auto log_message = "Saving config file " + config_path;
    if (write_logs) {
        LOGGER_I << log_message;
    } else {
        logs.push_back(std::make_tuple(Log_Level::info, log_message));
    }

    Configurations::save(*stream);

    log_message = "Saved config file " + config_path;
    if (write_logs) {
        LOGGER_I << log_message;
    } else {
        logs.push_back(std::make_tuple(Log_Level::info, log_message));
    }
}

bool User_Data_Folder::try_to_save_config() {
    try {
        save_config();
        return true;
    } catch (const config_exception& e) {
        LOGGER_E << "Error while saving config file - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error saving config file - message: " << e.what();
    }

    return false;
}

bool User_Data_Folder::save(std::string filename, Save_File& save_file) {
    cleanup_save_filename(filename);
    try {
        auto stream = filesystem.open_ofstream(filename, std::ios::out | std::ios::binary);
        if (!stream || !*stream) {
            throw file_loading_exception("Unable to open save file for writing: " + filename);
        }
        stream->exceptions(std::ios_base::failbit | std::ios_base::badbit);
        *stream << save_file;
        LOGGER_I << "Saved file " << filename;
        return save_file.is_valid();
    } catch (const std::ios_base::failure& e) {
        LOGGER_E << "Error saving file " << filename << " - error code: " << e.code() << " - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error saving file " << filename << " - message: " << e.what();
    }

    return false;
}

bool User_Data_Folder::load(std::string filename, Save_File& save_file) {
    cleanup_save_filename(filename);

    try {
        auto stream = filesystem.open_ifstream(filename, std::ios::in | std::ios::binary);
        if (!stream || !*stream) {
            throw file_loading_exception("Unable to open save file for reading: " + filename);
        }

        stream->exceptions(std::ios_base::failbit | std::ios_base::badbit);
        *stream >> save_file;
        LOGGER_I << "Loaded file " << filename;
        return save_file.is_valid();
    } catch (const std::ios_base::failure& e) {
        LOGGER_E << "Error loading file " << filename << " - error code: " << e.code() << " - message: " << e.what();
    } catch (const std::runtime_error& e) {
        LOGGER_E << "Error loading file " << filename << " - message: " << e.what();
    }

    return false;
}

std::string User_Data_Folder::get_keymap_filename() const {
    std::string filename = Configurations::get<std::string>("controls.mapping-file");
    if (filesystem.is_absolute_path(filename) || version_path.empty()) return filename;

    return version_path + filesystem.filename_component(filename);
}

bool User_Data_Folder::load_keymap_file(Key_Binder& key_binder) {
    // Always load the default keymap file first to register new virtual key names
    auto default_filename = Configurations::get<std::string>("controls.mapping-file");
    Readable_Filesystem* default_fs = file_utilities::game_data_filesystem();
    auto default_input = default_fs->open_ifstream(default_filename);
    if (!default_input || !*default_input) {
        LOGGER_W << "Couldn't read key default mapping file \"" << default_filename
            << "\", using default key mapping.";
        return false;
    }

    LOGGER_I << "Processing default keymap file " << default_filename;
    if (!key_binder.process_keymap_file(*default_input)) return false;

    // User key map file won't be created if it's the same as the default one
    auto user_filename = get_keymap_filename();
    if (!filesystem.exists(user_filename) || default_filename == user_filename) return true;

    // Load the user key map file and overwrite the default values
    Readable_Filesystem* user_fs = &filesystem;
    auto user_input = user_fs->open_ifstream(user_filename);
    if (!user_input || !*user_input) {
        LOGGER_W << "Couldn't read key user mapping file \"" << user_filename
            << "\", using default mapping file values.";
        return false;
    }

    LOGGER_I << "Processing keymap file " << user_filename;
    return key_binder.process_keymap_file(*user_input);
}

bool User_Data_Folder::save_keymap_file(Key_Binder& key_binder) {
    if (!key_binder.can_save()) return true;

    auto filename = get_keymap_filename();
    auto output = filesystem.open_ofstream(filename);
    if (!output || !*output) {
        LOGGER_E << "Unable to open keymap file " << filename << " for writing";
        return false;
    }

    LOGGER_I << "Saving keymap file " << filename;

    auto saved = key_binder.save_keymap_file(*output);
    if (saved) {
        LOGGER_I << "Finished saving keymap file " << filename;
    }

    return saved;
}

void User_Data_Folder::cleanup_save_filename(std::string& filename) const {
    string_utilities::normalize_slashes(filename);
    if (filename.find('/') == std::string::npos) {
        filename = version_path + filename;
    }
}