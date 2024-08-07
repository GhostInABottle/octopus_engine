#ifndef HPP_PHYSFS_FILESYSTEM
#define HPP_PHYSFS_FILESYSTEM

#include <string_view>
#include "readable_filesystem.hpp"

// PhysicsFS archive-based filesystem
class PhysFS_Filesystem : public Readable_Filesystem {
public:
    PhysFS_Filesystem(std::string_view arg, std::string_view archive_name);
    ~PhysFS_Filesystem();
    virtual std::unique_ptr<std::istream> open_ifstream(std::string filename, std::ios_base::openmode mode = std::ios_base::in) override;
    virtual std::unique_ptr<std::istream> open_binary_ifstream(std::string filename) override;
    virtual std::string read_file(std::string filename) override;
    virtual bool exists(const std::string& path) override;
    virtual bool is_regular_file(const std::string& path) override;
    virtual bool is_directory(const std::string& path) override;
    virtual std::tuple<unsigned long long, std::tm> last_write_time(const std::string& path) override;
    virtual std::uintmax_t file_size(const std::string& path) override;
    virtual bool is_absolute_path(const std::string& path) override;
    virtual std::string filename_component(const std::string& path) override;
    virtual std::string stem_component(const std::string& path) override;
    virtual std::string extension(const std::string& path) override;
    virtual std::vector<std::string> directory_content_names(const std::string& path) override;
    virtual std::vector<Path_Info> directory_content_details(const std::string& path) override;
};

#endif
