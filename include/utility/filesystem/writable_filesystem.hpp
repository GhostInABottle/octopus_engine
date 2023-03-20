#ifndef HPP_WRITABLE_FILESYSTEM
#define HPP_WRITABLE_FILESYSTEM

#include "readable_filesystem.hpp"

// Utilities for modifying a filesystem and writing files
class Writable_Filesystem : public Readable_Filesystem {
public:
    // Open the file with the given UTF8 name for writing
    virtual std::ofstream open_ofstream(std::string filename, std::ios_base::openmode mode = std::ios_base::out) = 0;
    // Copy a file, returns a boolean indicating success
    virtual bool copy_file(const std::string& source, const std::string& destination) = 0;
    // Remove a file, returns a boolean indicating success
    virtual bool remove_file(const std::string& filename) = 0;
    // Create one or more directories specified in the path
    virtual bool create_directories(const std::string& path) = 0;
};

#endif
