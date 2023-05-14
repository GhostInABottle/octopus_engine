#include "../../../include/filesystem/user_data_folder.hpp"
#include "../../../include/configurations.hpp"
#include "../../../include/log.hpp"
#include "../../../include/save_file.hpp"
#include "../../../include/key_binder.hpp"
#include "../../../include/exceptions.hpp"
#include "../../../include/utility/string.hpp"
#include "../../../include/utility/file.hpp"
#include "../../../include/vendor/platform_folders.hpp"
#include <stdexcept>

User_Data_Folder::User_Data_Folder(Writable_Filesystem& filesystem) : filesystem(filesystem), parsed_default_config(false) {
    std::string default_folder;
    auto add_game_folder = false;

    if (!Configurations::defaults_loaded()) {
        parse_config();
    }

    auto config_folder = Configurations::get<std::string>("game.data-folder");
    string_utilities::trim(config_folder);

    if (config_folder.empty()) {
        try {
            default_folder = sago::getSaveGamesFolder1();
            add_game_folder = true;
        } catch (std::runtime_error& e) {
            warnings.push_back("Unable to retrieve default save folder. Using executable folder - "
                + std::string{ e.what() });
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

    base_path = default_folder;

    if (add_game_folder) {
        auto title = Configurations::get<std::string>("game.title");
        if (title.empty()) {
            title = "OctopusEngine";
        }
        default_folder += title + "/";
        game_path = default_folder;

        auto data_dir_version = Configurations::get<std::string>("game.data-folder-version");
        default_folder += data_dir_version + "/";
    } else {
        game_path = default_folder;
    }

    version_path = default_folder;

    // Create the folder if needed
    auto created = filesystem.create_directories(default_folder);
    if (created) return;

    warnings.push_back("Unable to create save folder. " + default_folder
        + " Using the executable's folder instead");

    base_path = "";
    game_path = "";
    version_path = "";
}

void User_Data_Folder::parse_config() {
    if (!parsed_default_config) {
        parse_default_config();
    }

    if (version_path.empty()) return;

    auto config_path = version_path + "config.ini";
    try {
        if (!filesystem.file_exists(config_path)) {
            save_config(true);
            return;
        }

        auto existing_file_stream = filesystem.open_ifstream(config_path);
        if (!existing_file_stream || !*existing_file_stream) {
            throw config_exception("Couldn't read file " + config_path);
        }

        warnings = Configurations::parse(*existing_file_stream);
        for (auto& warning : warnings) {
            LOGGER_W << warning;
        }
    } catch (config_exception& e) {
        LOGGER_E << "Unable to write config file to " << version_path << ": " << e.what();
    }
}

void User_Data_Folder::parse_default_config() {
    if (parsed_default_config) return;

    auto config_fs = file_utilities::default_config_filesystem();
    auto default_stream = config_fs->open_ifstream("config.ini");
    if (!default_stream || !*default_stream) {
        throw config_exception("Couldn't read file config.ini");
    }

    auto warnings = Configurations::parse(*default_stream);
    LOGGER_I << "Parsed the default config.ini file";

    for (auto& warning : warnings) {
        LOGGER_W << warning;
    }

    parsed_default_config = true;
}

void User_Data_Folder::save_config(bool force) {
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

    LOGGER_I << "Saving config file " << config_path;

    Configurations::save(*stream);

    LOGGER_I << "Saved config file " << config_path;
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

    return version_path + filesystem.get_filename_component(filename);
}

bool User_Data_Folder::load_keymap_file(Key_Binder& key_binder) {
    std::string filename = get_keymap_filename();
    Readable_Filesystem* fs = &filesystem;
    if (!filesystem.file_exists(filename)) {
        filename = Configurations::get<std::string>("controls.mapping-file");
        fs = file_utilities::game_data_filesystem();
    }

    auto input = fs->open_ifstream(filename);
    if (!input || !*input) {
        LOGGER_W << "Couldn't read key mapping file \"" << filename << "\", using default key mapping.";
        return false;
    }

    LOGGER_I << "Processing keymap file " << filename;
    return key_binder.process_keymap_file(*input);
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

void User_Data_Folder::cleanup_save_filename(std::string& filename) {
    string_utilities::normalize_slashes(filename);
    if (filename.find('/') == std::string::npos) {
        filename = version_path + filename;
    }
}