#ifndef HPP_DISK_FILESYSTEM
#define HPP_DISK_FILESYSTEM

#include "writable_filesystem.hpp"

// A disk-based filesystem (such as the standard library's or boost's)
class Disk_Filesystem : public Writable_Filesystem {
    virtual std::unique_ptr<std::istream> open_ifstream(std::string filename, std::ios_base::openmode mode = std::ios_base::in) override;
    virtual std::unique_ptr<std::istream> open_binary_ifstream(std::string filename) override;
    virtual std::unique_ptr<std::ostream> open_ofstream(std::string filename, std::ios_base::openmode mode) override;
    virtual std::string read_file(std::string filename) override;
    virtual std::vector<std::string> directory_content_names(const std::string& path) override;
    virtual std::vector<Path_Info> directory_content_details(const std::string& path) override;
    // Get the file names/details without checking for exceptions, used to implement the versions above
    virtual std::vector<std::string> directory_content_names_unchecked(const std::string& path) = 0;
    virtual std::vector<Path_Info> directory_content_details_unchecked(const std::string& path) = 0;
};

#endif
