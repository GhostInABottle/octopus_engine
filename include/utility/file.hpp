#ifndef HPP_UTILITY_FILE
#define HPP_UTILITY_FILE

#include <string>
#include <vector>
#include <fstream>

namespace file_utilities {
    // Open the file with the given UTF8 name for reading
    std::ifstream open_ifstream(std::string filename, std::ios_base::openmode mode = std::ios_base::in);
    // Open the file with the given UTF8 name for writing
    std::ofstream open_ofstream(std::string filename, std::ios_base::openmode mode = std::ios_base::out);
    // Check if a file exists
    bool file_exists(const std::string& filename);
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
    // Check if path is a regular file (not directory or any other type of file)
    bool is_regular_file(const std::string& path);
    // Check if path is a directory
    bool is_directory(const std::string& path);
    // Get file names in a directory
    std::vector<std::string> list_directory_files(const std::string& path);
    // Copy a file, returns a boolean indicating success
    bool copy_file(const std::string& source, const std::string& destination);
    // Remove a file, returns a boolean indicating success
    bool remove_file(const std::string& filename);
    // Check if a path is absolute
    bool is_absolute_path(const std::string& path);
    // Get filename component of a path (with extension)
    std::string get_filename_component(const std::string& path);
    // Get stem component of a path (filename without extension)
    std::string get_stem_component(const std::string& path);
}

#endif