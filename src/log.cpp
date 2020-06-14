#include "../include/log.hpp"
#include "../include/configurations.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/string.hpp"

Log_Level Log::reporting_level = Log_Level::debug;
std::ofstream Log::log_file;
bool Log::enabled = false;

void Log::open_log_file() {
    if (Configurations::defaults_loaded()) {
        enabled = Configurations::get<bool>("logging.enabled");
    }
    if (!enabled)
        return;
    auto filename = Configurations::get<std::string>("logging.filename");
    if (filename.empty()) filename = "game.log";
    normalize_slashes(filename);
    auto data_dir = get_data_directory(false);
    if (filename.find("/") == std::string::npos) {
        filename = data_dir + filename;
    }
    int mode = static_cast<int>(std::fstream::out);
    auto config_mode = Configurations::get<std::string>("logging.mode");
    if (config_mode == "truncate")
        mode |= std::ios_base::trunc;
    else if (config_mode == "append")
        mode |= std::ios_base::app;
    log_file.open(filename.c_str(), mode);
    enabled = static_cast<bool>(log_file);
    std::string config_level = Configurations::get<std::string>("logging.level");
    capitalize(config_level);
    reporting_level = log_level_from_string(config_level);
}