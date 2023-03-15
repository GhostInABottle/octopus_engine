#include "../../../include/utility/filesystem/readable_filesystem.hpp"
#include "../../../include/utility/string.hpp"

void file_utilities::Readable_Filesystem::normalize_slashes(std::string& filename) {
    string_utilities::replace_all(filename, "\\", "/");
}
