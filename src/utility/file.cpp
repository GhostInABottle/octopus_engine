#include "../../include/utility/file.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/configurations.hpp"
#include "../../include/log.hpp"
#include "../../include/vendor/platform_folders.hpp"
#include <stdexcept>
#include <streambuf>
#include <boost/algorithm/string.hpp>
#ifdef OCB_USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
#ifdef _WIN32
#include "../../include/vendor/utf8conv.h"
#endif

fs::path string_to_utf8_path(const std::string& str) {
#ifdef OCB_USE_BOOST_FILESYSTEM
#ifdef _WIN32
    return fs::path(win32::Utf8ToUtf16(str));
#else
    return fs::path(str)
#endif
#else
    return fs::u8path(str);
#endif
}

std::ifstream file_utilities::open_ifstream(std::string filename, std::ios_base::openmode mode) {
    normalize_slashes(filename);
#ifdef _WIN32
    std::ifstream stream{ win32::Utf8ToUtf16(filename), mode };
#else
    std::ifstream stream{ filename, mode };
#endif

    return stream;
}

std::ofstream file_utilities::open_ofstream(std::string filename, std::ios_base::openmode mode) {
    normalize_slashes(filename);
#ifdef _WIN32
    std::ofstream stream{ win32::Utf8ToUtf16(filename) , mode };
#else
    std::ofstream stream{ filename, mode };
#endif

    return stream;
}

bool file_utilities::file_exists(const std::string& filename) {

    return fs::exists(string_to_utf8_path(filename));
}

std::string file_utilities::read_file(std::string filename) {
    auto stream = open_ifstream(filename);
    if (!stream)
        throw std::runtime_error("Couldn't open file for reading: " + filename);

    return std::string((std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>());
}

void file_utilities::normalize_slashes(std::string& filename) {
    string_utilities::replace_all(filename, "\\", "/");
}

std::string file_utilities::get_data_directory(bool log_errors) {
    std::string default_folder;
    auto add_game_folder = false;
    auto config_folder = Configurations::get<std::string>("game.data-folder");
    string_utilities::trim(config_folder);
    if (config_folder.empty()) {
        try {
            default_folder = sago::getSaveGamesFolder1();
            add_game_folder = true;
        } catch (std::runtime_error& e) {
            if (log_errors) {
                LOGGER_W << "Unable to retrieve default save folder. Using executable folder - " << e.what();
            }
        }
    } else {
        default_folder = config_folder;
        if (default_folder == ".") {
            default_folder = "";
        } else if (default_folder.rfind("./", 0) == 0) {
            default_folder = default_folder.substr(2);
        }
    }
    
    if (default_folder.empty()) {
        return default_folder;
    }
    normalize_slashes(default_folder);
    if (default_folder.back() != '/') {
        default_folder += '/';
    }
    if (add_game_folder) {
        auto title = Configurations::get<std::string>("game.title");
        if (title.empty()) title = "OctopusEngine";
        auto data_dir_version = Configurations::get<std::string>("game.data-folder-version");
        default_folder += title + "/" + data_dir_version + "/";
    }

    // Create the folder if needed
    try {
        auto utf8_path = string_to_utf8_path(default_folder);
        if (!fs::exists(utf8_path)) {
            fs::create_directories(utf8_path);
        }
    } catch (fs::filesystem_error& e) {
        if (log_errors) {
            LOGGER_W << "Unable to create save folder. Using executable folder - " << e.what();
        }
        default_folder = "";
    }

    return default_folder;
}

void file_utilities::parse_config(const std::string& filename) {
    auto warnings = Configurations::parse(filename);
    for (auto& warning : warnings) {
        LOGGER_W << warning;
    }
    auto data_dir = get_data_directory();
    if (data_dir.empty()) return;

    auto config_path = data_dir + filename;
    try {
        if (file_exists(config_path)) {
            warnings = Configurations::parse(config_path);
            for (auto& warning : warnings) {
                LOGGER_W << warning;
            }
        } else {
            Configurations::save(config_path);
        }
    } catch (config_exception& e) {
        LOGGER_E << "Unable to write config file to " << data_dir << ": " << e.what();
    }
}

void file_utilities::save_config(const std::string& filename) {
    if (!Configurations::changed() || !Configurations::get<bool>("debug.update-config-files")) {
        return;
    }
    auto data_dir = get_data_directory();
    auto config_path = data_dir + filename;
    Configurations::save(config_path);
    LOGGER_I << "Saved config file " << config_path;
}

bool file_utilities::is_regular_file(const std::string& path) {
    try {
        return fs::is_regular_file(string_to_utf8_path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error while trying to check if path is a regular file:" << path << ", assuming it isn't - " << e.what();
    }
    return false;
}

bool file_utilities::is_directory(const std::string& path) {
    try {
        return fs::is_directory(string_to_utf8_path(path));
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error while trying to check if path is a directory:" << path << ", assuming it isn't - " << e.what();
    }
    return false;
}

std::vector<std::string> file_utilities::list_directory_files(const std::string& path) {
    std::vector<std::string> result;
    try {
        auto utf8_path = string_to_utf8_path(path);
        if (!fs::is_directory(utf8_path)) {
            LOGGER_E << "Tried to list a path that is not a directory " << path;
            return result;
        }
        for (auto& p : fs::directory_iterator(utf8_path)) {
            if (fs::is_regular_file(p) || fs::is_directory(p)) {
                result.push_back(p.path().filename().string());
            }
        }
    } catch (fs::filesystem_error& e) {
        LOGGER_E << "Error while trying to list directory " << path << ", some or all files might be ignored - " << e.what();
    }
    return result;
}

bool file_utilities::copy_file(const std::string& source, const std::string& destination) {
    try {
#ifdef OCB_USE_BOOST_FILESYSTEM
        fs::copy_file(string_to_utf8_path(source), string_to_utf8_path(destination),
            fs::copy_option::overwrite_if_exists);
#else
        fs::copy(string_to_utf8_path(source), string_to_utf8_path(destination),
            fs::copy_options::overwrite_existing);
#endif
        return true;
    } catch (fs::filesystem_error & e) {
        LOGGER_E << "Error copying " << source << " to " << destination << ": " << e.what();
        return false;
    }
}

bool file_utilities::remove_file(const std::string& filename) {
    try {
        return fs::remove(string_to_utf8_path(filename));
    } catch (fs::filesystem_error & e) {
        LOGGER_E << "Error deleting file " << filename << ": " << e.what();
        return false;
    }
}

bool file_utilities::is_absolute_path(const std::string& path_str) {
    return fs::path{ string_to_utf8_path(path_str) }.is_absolute();
}

std::string file_utilities::get_filename_component(const std::string& path_str) {
    return fs::path{ string_to_utf8_path(path_str) }.filename().string();
}

std::string file_utilities::get_stem_component(const std::string& path_str) {
    return fs::path{ string_to_utf8_path(path_str) }.stem().string();
}