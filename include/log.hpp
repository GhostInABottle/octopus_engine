#ifndef HPP_LOG
#define HPP_LOG

#include <fstream>
#include <sstream>
#include <string>
#include "utility/string.hpp"

// Ignore 'inheritance by dominance' warning
#pragma warning(push)
#pragma warning(disable: 4250)
// Levels of log messages
enum class Log_Level { error, warning, info, debug };

// A simple logging class. Usage:
// Log(level).lvalue() << stuff
// LOGGER(level) << stuff
// LOGGER_L << stuff
class Log : public std::stringstream {
public:
    // Constructor: sets the level
    explicit Log(Log_Level level = Log_Level::info) : current_level(level) {
        if (!log_file.is_open()) {
            open_log_file();
        }
    }
    // Destructor: writes the message to file
    ~Log() {
        if (!enabled || current_level > Log::get_reporting_level())
            return;
        log_file << "- " << timestamp() << " " << log_level_to_string(current_level)
            << ": " << this->str() << std::endl;
        if (!log_file) {
            enabled = false;
        }
    }
    // To avoid rvalue issues in stringstream(e.g. char* being treated as void*)
    Log& lvalue() {
        return *this;
    }
    // Get the reporting level
    static Log_Level get_reporting_level() {
        return reporting_level;
    }
    // Set the reporting level
    static void set_reporting_level(Log_Level level) {
        reporting_level = level;
    }
    // Convert an error level enum value to string
    static std::string log_level_to_string(Log_Level level) {
        switch(level) {
        case Log_Level::error:
            return "ERROR";
        case Log_Level::warning:
            return "WARNING";
        case Log_Level::info:
            return "INFO";
        case Log_Level::debug:
            return "DEBUG";
        }
        return "";
    }
    // Convert a string into an error level enum value
    static Log_Level log_level_from_string(const std::string& level) {
        if (level == "ERROR")
            return Log_Level::error;
        else if (level == "WARNING")
            return Log_Level::warning;
        else if (level == "INFO")
            return Log_Level::info;
        else if (level == "DEBUG")
            return Log_Level::debug;
        return Log_Level::info;
    }
private:
    // The reporting level of the current message
    Log_Level current_level;
    // The global reporting level
    static Log_Level reporting_level;
    // The file to write into
    static std::ofstream log_file;
    // Is logging disabled? (e.g. failed to open log file)
    static bool enabled;
    // Open the log file for the first time
    static void open_log_file();
};

#pragma warning(pop)

// Some macros to ease using the logger
#define LOGGER(level) \
    if (level <= Log::get_reporting_level()) Log(level).lvalue()
#define LOGGER_E LOGGER(Log_Level::error)
#define LOGGER_W LOGGER(Log_Level::warning)
#define LOGGER_I LOGGER(Log_Level::info)
#define LOGGER_D LOGGER(Log_Level::debug)

#endif
