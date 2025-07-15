#include "configurations.hpp"
#include "environments/environment.hpp"
#include "log.hpp"
#include "utility/file.hpp"
#include "utility/string.hpp"
#include <iostream>

namespace detail {
    static void rotate_files(std::string filename, Writable_Filesystem& fs) {
        int file_count = Configurations::get<int>("logging.file-count");
        int max_size = Configurations::get<int>("logging.max-file-size-kb");

        bool rolling = file_count > 1
            && max_size > 0
            && fs.exists(filename)
            && fs.file_size(filename) / 1024 > max_size;
        if (!rolling) return;

        std::string folder{""};
        const auto last_slash = filename.find_last_of('/');
        if (last_slash != std::string::npos) {
            folder = filename.substr(0, last_slash + 1);
        }

        const auto ext = fs.extension(filename);
        const auto stem = fs.stem_component(filename);

        const auto filename_prefix = folder + stem + "_";
        // Copy each file to the next slot (e.g. file_3.log to file_4.log)
        for (int i = file_count; i >= 2; --i) {
            const auto source_file = filename_prefix + std::to_string(i) + ext;
            if (!fs.exists(source_file)) continue;

            const auto dest_file = filename_prefix + std::to_string(i + 1) + ext;
            fs.copy(source_file, dest_file);
        }

        // Copy the main log file to slot 2 (e.g. file.log to file_2.log)
        if (fs.copy(filename, filename_prefix + "2" + ext)) {
            fs.remove(filename);
        }

        // Delete the oldest file past the max count (e.g. file_6.log is the count is 5)
        const auto extra_file = filename_prefix + std::to_string(file_count + 1) + ext;
        if (fs.exists(extra_file)) {
            fs.remove(extra_file);
        }
    }
}

Log_Level Log::reporting_level = Log_Level::debug;
std::unique_ptr<std::ostream> Log::log_file;
std::ostream* Log::log_fallback = &std::cerr;
bool Log::log_file_opened = false;
bool Log::enabled = true;
const Environment* Log::environment = nullptr;

void Log::open_log_file() {
    if (enabled && Configurations::defaults_loaded()) {
        enabled = Configurations::get<bool>("logging.enabled");
    } else {
        // Use fallback
        reporting_level = Log_Level::warning;
        enabled = true;
        return;
    }

    if (!enabled) return;

    // Disable logging while opening the log file
    enabled = false;

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

    auto& filesystem = data_folder
        ? data_folder->get_filesystem()
        : *file_utilities::disk_filesystem();

    auto config_mode = Configurations::get<std::string>("logging.mode");
    int mode = static_cast<int>(std::ios_base::out);
    if (config_mode == "append") {
        mode |= std::ios_base::app;
        detail::rotate_files(filename, filesystem);
    } else {
        mode |= std::ios_base::trunc;
    }

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

    enabled = true;
}
