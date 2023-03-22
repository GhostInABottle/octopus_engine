#include "../include/log.hpp"
#include "../include/configurations.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/string.hpp"

Log_Level Log::reporting_level = Log_Level::debug;
std::unique_ptr<std::ostream> Log::log_file;
bool Log::enabled = false;

void Log::open_log_file() {
    if (Configurations::defaults_loaded()) {
        enabled = Configurations::get<bool>("logging.enabled");
    }
    if (!enabled) return;

    auto filename = Configurations::get<std::string>("logging.filename");
    if (filename.empty()) filename = "game.log";

    string_utilities::normalize_slashes(filename);
    auto data_folder = file_utilities::user_data_folder();
    auto data_dir = data_folder->get_version_path();
    if (filename.find("/") == std::string::npos) {
        filename = data_dir + filename;
    }

    int mode = static_cast<int>(std::ios_base::out);
    auto config_mode = Configurations::get<std::string>("logging.mode");
    if (config_mode == "truncate")
        mode |= std::ios_base::trunc;
    else if (config_mode == "append")
        mode |= std::ios_base::app;

    log_file = data_folder->get_filesystem().open_ofstream(filename, static_cast<std::ios_base::openmode>(mode));
    enabled = static_cast<bool>(log_file);
    std::string config_level = Configurations::get<std::string>("logging.level");
    string_utilities::capitalize(config_level);
    reporting_level = log_level_from_string(config_level);
}
