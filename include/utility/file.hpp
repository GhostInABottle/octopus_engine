#ifndef HPP_UTILITY_FILE
#define HPP_UTILITY_FILE

#include <string>
#include <vector>

// Check if a file exists
bool file_exists(std::string filename);
// Read file content into a string
std::string read_file(std::string filename);
// Change backslashes to forward slashes
void normalize_slashes(std::string& filename);
// Get directory for writing game data (saves, config, logs, etc.)
std::string get_data_directory(bool log_errors = true);
// Parse config file and save it to data directory if needed
void parse_config(const std::string& filename);
// Save config file if it changed since last save
void save_config(const std::string& filename);
// Get file names in a directory
std::vector<std::string> list_directory_files(std::string path);
// Copy a file, returns a boolean indicating success
bool copy_file(std::string source, std::string destination);
// Remove a file, returns a boolean indicating success
bool remove_file(std::string filename);


#endif