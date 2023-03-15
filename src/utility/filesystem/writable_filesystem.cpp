#include "../../../include/utility/filesystem/writable_filesystem.hpp"
#include "../../../include/configurations.hpp"
#include "../../../include/log.hpp"
#include "../../../include/utility/string.hpp"
#include "../../../include/vendor/platform_folders.hpp"
#include <stdexcept>

std::string file_utilities::Writable_Filesystem::get_data_directory(bool log_errors) {
    std::string default_folder;
    auto add_game_folder = false;

    auto config_folder = Configurations::get<std::string>("game.data-folder");
    string_utilities::trim(config_folder);

    if (config_folder.empty()) {
        try {
            default_folder = sago::getSaveGamesFolder1();
            add_game_folder = true;
        } catch (std::runtime_error& e) {
            if (log_errors) {
                LOGGER_W << "Unable to retrieve default save folder. Using executable folder - " << e.what();
            }
        }
    } else {
        default_folder = config_folder;
        if (default_folder == ".") {
            default_folder = "";
        } else if (default_folder.rfind("./", 0) == 0) {
            default_folder = default_folder.substr(2);
        }
    }

    if (default_folder.empty()) {
        return default_folder;
    }

    normalize_slashes(default_folder);
    if (default_folder.back() != '/') {
        default_folder += '/';
    }

    if (add_game_folder) {
        auto title = Configurations::get<std::string>("game.title");
        if (title.empty()) title = "OctopusEngine";
        auto data_dir_version = Configurations::get<std::string>("game.data-folder-version");
        default_folder += title + "/" + data_dir_version + "/";
    }

    // Create the folder if needed
    auto created = create_directories(default_folder);
    if (created) return default_folder;

    if (log_errors) {
        LOGGER_W << "Unable to create save folder. Using the executable's folder instead";
    }

    return "";
}
