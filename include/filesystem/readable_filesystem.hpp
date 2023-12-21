#ifndef HPP_READABLE_FILESYSTEM
#define HPP_READABLE_FILESYSTEM

#include <string>
#include <vector>
#include <tuple>
#include <iosfwd>
#include <ios>
#include <ctime>
#include <memory>
#include "path_info.hpp"

// Utilities for examining a filesystem and reading files
class Readable_Filesystem {
public:
    virtual ~Readable_Filesystem() {}
    // Open the file with the given UTF8 name for reading
    virtual std::unique_ptr<std::istream> open_ifstream(std::string filename, std::ios_base::openmode mode = std::ios_base::in) = 0;
    virtual std::unique_ptr<std::istream> open_binary_ifstream(std::string filename) = 0;
    // Check if a file or folder exists
    virtual bool exists(const std::string& path) = 0;
    // Read file content into a string
    virtual std::string read_file(std::string filename) = 0;
    // Check if path is a regular file (not directory or any other type of file)
    virtual bool is_regular_file(const std::string& path) = 0;
    // Check if path is a directory
    virtual bool is_directory(const std::string& path) = 0;
    // Get the last modification time (milliseconds and std::time)
    virtual std::tuple<unsigned long long, std::tm> last_write_time(const std::string& path) = 0;
    // Get file names in a directory
    virtual std::vector<std::string> directory_content_names(const std::string& path) = 0;
    // Get file information in a directory
    virtual std::vector<Path_Info> directory_content_details(const std::string& path) = 0;
    // Check if a path is absolute
    virtual bool is_absolute_path(const std::string& path) = 0;
    // Get filename component of a path (with extension)
    virtual std::string get_filename_component(const std::string& path) = 0;
    // Get stem component of a path (filename without extension)
    virtual std::string get_stem_component(const std::string& path) = 0;
};

#endif
