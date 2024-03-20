#include "../include/configurations.hpp"
#include "../include/environments/environment.hpp"
#include "../include/log.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/string.hpp"
#include <iostream>

Log_Level Log::reporting_level = Log_Level::debug;
std::unique_ptr<std::ostream> Log::log_file;
std::ostream* Log::log_fallback = &std::cerr;
bool Log::log_file_opened = false;
bool Log::enabled = false;
const Environment* Log::environment = nullptr;

void Log::open_log_file() {
    if (Configurations::defaults_loaded()) {
        enabled = Configurations::get<bool>("logging.enabled");
    } else {
        // Use fallback
        reporting_level = Log_Level::warning;
        enabled = true;
        return;
    }

    if (!enabled) return;

    auto filename = Configurations::get<std::string>("logging.filename");
    if (filename.empty()) filename = "game.log";

    string_utilities::normalize_slashes(filename);
    std::shared_ptr<User_Data_Folder> data_folder;
    if (environment) {
        data_folder = file_utilities::user_data_folder(*environment);
        auto data_dir = data_folder->get_version_path();
        if (filename.find("/") == std::string::npos) {
            filename = data_dir + filename;
        }
    }

    int mode = static_cast<int>(std::ios_base::out);
    auto config_mode = Configurations::get<std::string>("logging.mode");
    if (config_mode == "truncate")
        mode |= std::ios_base::trunc;
    else if (config_mode == "append")
        mode |= std::ios_base::app;

    auto& filesystem = data_folder
        ? data_folder->get_filesystem()
        : *file_utilities::disk_filesystem();

    log_file = filesystem.open_ofstream(filename, static_cast<std::ios_base::openmode>(mode));
    log_file_opened = static_cast<bool>(log_file);

    std::string config_level = Configurations::get<std::string>("logging.level");
    string_utilities::capitalize(config_level);
    reporting_level = log_level_from_string(config_level);

    // Listen to config changes
    Configurations::add_observer("Log",
        [](const std::string& key) {
            if (key != "logging.level") return;

            auto config_level = Configurations::get<std::string>(key);
            string_utilities::capitalize(config_level);
            Log::set_reporting_level(Log::log_level_from_string(config_level));
        }
    );
}
