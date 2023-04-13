#ifndef OCB_USE_BOOST_FILESYSTEM
#ifndef HPP_STANDARD_FILESYSTEM
#define HPP_STANDARD_FILESYSTEM

#include "disk_filesystem.hpp"

// Implementation of filesystem utilities using the standard <filesystem> library
// Not available on older versions (<8) of GCC, where we define OCB_USE_BOOST_FILESYSTEM
class Standard_Filesystem : public Disk_Filesystem {
    virtual bool file_exists(const std::string& filename) override;
    virtual bool is_regular_file(const std::string& path) override;
    virtual bool is_directory(const std::string& path) override;
    virtual std::tuple<unsigned long long, std::tm> last_write_time(const std::string& path) override;
    virtual std::vector<std::string> directory_content_names_unchecked(const std::string& path) override;
    virtual std::vector<Path_Info> directory_content_details_unchecked(const std::string& path) override;
    virtual bool is_absolute_path(const std::string& path) override;
    virtual std::string get_filename_component(const std::string& path) override;
    virtual std::string get_stem_component(const std::string& path) override;
    virtual bool copy_file(const std::string& source, const std::string& destination) override;
    virtual bool remove_file(const std::string& filename) override;
    virtual bool create_directories(const std::string& path) override;
};

#endif
#endif
