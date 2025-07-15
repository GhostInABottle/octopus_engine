#ifdef OCB_USE_BOOST_FILESYSTEM
#ifndef HPP_BOOST_FILESYSTEM
#define HPP_BOOST_FILESYSTEM

#include "disk_filesystem.hpp"

// Boost <boost/filesystem.hpp> library implementation
// Only used on older versions (<8) of GCC, where we define OCB_USE_BOOST_FILESYSTEM
class Boost_Filesystem : public Disk_Filesystem {
    virtual bool exists(const std::string& path) override;
    virtual bool is_regular_file(const std::string& path) override;
    virtual bool is_directory(const std::string& path) override;
    virtual std::tuple<unsigned long long, std::tm> last_write_time(const std::string& path) override;
    virtual std::uintmax_t file_size(const std::string& path) override;
    virtual std::vector<std::string> directory_content_names_unchecked(const std::string& path) override;
    virtual std::vector<Path_Info> directory_content_details_unchecked(const std::string& path) override;
    virtual bool is_absolute_path(const std::string& path) override;
    virtual std::string filename_component(const std::string& path) override;
    virtual std::string stem_component(const std::string& path) override;
    virtual std::string extension(const std::string& path) override;
    virtual bool copy(const std::string& source, const std::string& destination) override;
    virtual bool remove(const std::string& path) override;
    virtual bool rename(const std::string& old_path, const std::string& new_path) override;
    virtual bool create_directories(const std::string& path) override;
};

#endif
#endif
