#include "../../include/filesystem/standard_filesystem.hpp"
#include "../../include/utility/file.hpp"
#if OCB_USE_BOOST_FILESYSTEM
    #include "../../include/filesystem/boost_filesystem.hpp"
#endif
#include "../../include/filesystem/physfs_filesystem.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/configurations.hpp"
#include "../../include/log.hpp"
#include <cstdlib>

#ifdef _WIN32
#include "../../include/vendor/utf8conv.h"
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <combaseapi.h>
#endif

namespace file_utilities::detail {
    static std::shared_ptr<Writable_Filesystem> disk_filesystem;
    static std::shared_ptr<Readable_Filesystem> virtual_filesystem;
    static std::string virtual_archive_name;
    static Writable_Filesystem* user_data_filesystem{ nullptr };
    static Readable_Filesystem* game_data_filesystem{ nullptr };
    static Readable_Filesystem* default_config_filesystem{ nullptr };
    static std::shared_ptr<User_Data_Folder> user_data_folder;
#ifdef _WIN32
    static bool open_windows_path(const std::string& path) {
        // It's good practice to initialize COM before calling ShellExecute
        // It fails if called in the wrong thread model, so we try both
        auto hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (hr == RPC_E_CHANGED_MODE) {
            hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        }

        // S_FALSE means it was already initialized
        if (hr != S_OK && hr != S_FALSE) {
            LOGGER_E << "Error while trying to co-initialize COM and open " << path << ": " << hr;
            return false;
        }

        std::wstring wide_path;
        try {
            wide_path = win32::Utf8ToUtf16(path);
        } catch (std::runtime_error& err) {
            LOGGER_E << "Failed to convert URL " << path << " to UTF16: " << err.what();
            CoUninitialize();
            return false;
        }

        auto result = ShellExecuteW(NULL, L"open", wide_path.c_str(), NULL, NULL, SW_SHOWNORMAL);

        CoUninitialize();

        // ShellExecute returns a value > 32 if it succeeds
        auto succeeded = result > reinterpret_cast<HINSTANCE>(32);
        if (!succeeded) {
            LOGGER_E << "ShellExecute failed to open " << path << " with result: " << result;
        }

        return succeeded;
    }
#endif
}

std::shared_ptr<Writable_Filesystem> file_utilities::disk_filesystem(std::string_view) {
    if (detail::disk_filesystem) return detail::disk_filesystem;

#ifdef OCB_USE_BOOST_FILESYSTEM
    detail::disk_filesystem = std::make_shared<Boost_Filesystem>();
#else
    detail::disk_filesystem = std::make_shared<Standard_Filesystem>();
#endif

    return detail::disk_filesystem;
}

std::shared_ptr<Readable_Filesystem> file_utilities::virtual_filesystem(std::string_view arg, std::string_view archive_name) {
    auto already_set = detail::virtual_filesystem
        && (archive_name.empty() || archive_name == detail::virtual_archive_name);
    if (already_set) {
        return detail::virtual_filesystem;
    }

    detail::virtual_filesystem = std::make_shared<PhysFS_Filesystem>(arg, archive_name);
    detail::virtual_archive_name = archive_name;

    return detail::virtual_filesystem;
}

Readable_Filesystem* file_utilities::game_data_filesystem(std::string_view arg) {
    if (detail::game_data_filesystem) return detail::game_data_filesystem;

    if (!Configurations::defaults_loaded()) {
        throw config_exception{"Trying to get game data FS but default configs were not loaded."
            " Make sure User_Data_Folder::parse_default_config is called"};
    }

    std::shared_ptr<Readable_Filesystem> fs = disk_filesystem(arg);
    auto archive_name = Configurations::get<std::string>("game.archive-path");
    if (!archive_name.empty()) {
        if (!fs->exists(archive_name)) {
            throw config_exception { "Configured archive file was not found in executable directory: " + archive_name };
        }
        fs = virtual_filesystem(arg, archive_name);
    }

    detail::game_data_filesystem = fs.get();
    return detail::game_data_filesystem;
}

void file_utilities::set_game_data_filesystem(Readable_Filesystem* fs) {
    detail::game_data_filesystem = fs;
}

Readable_Filesystem* file_utilities::default_config_filesystem(std::string_view arg) {
    if (detail::default_config_filesystem) return detail::default_config_filesystem;

    // Check if config.ini exists in the same physical directory as the executable
    auto disk_fs = disk_filesystem(arg);
    if (disk_fs->exists("config.ini")) {
        detail::default_config_filesystem = disk_fs.get();
        return detail::default_config_filesystem;
    }

    // Try to find config.ini in any default archive
    std::string default_archive_names[] = {
        "game", "octopus", "octopus_engine"
    };
    std::string default_archive_extensions[] = {
        ".dat", ".bin", ".zip", ".ocd"
    };

    for (auto& name : default_archive_names) {
        for (auto& ext : default_archive_extensions) {
            auto archive_name = name + ext;
            if (!disk_fs->exists(archive_name)) continue;
            auto vfs = virtual_filesystem(arg, archive_name);
            if (!vfs->exists("config.ini")) {
                throw config_exception{ "Unable to find config.ini file in archive " + archive_name };
            }
            detail::default_config_filesystem = vfs.get();
            return detail::default_config_filesystem;
        }
    }

    throw config_exception{ "Unable to find config.ini file" };
}

void file_utilities::set_default_config_filesystem(Readable_Filesystem* fs) {
    detail::default_config_filesystem = fs;
}

Writable_Filesystem* file_utilities::user_data_filesystem(std::string_view arg) {
    if (detail::user_data_filesystem) return detail::user_data_filesystem;

    auto disk_fs = disk_filesystem();
    detail::user_data_filesystem = disk_fs.get();
    return detail::user_data_filesystem;
}

void file_utilities::set_user_data_filesystem(Writable_Filesystem* fs) {
    detail::user_data_filesystem = fs;
}

std::shared_ptr<User_Data_Folder> file_utilities::user_data_folder(const Environment& env) {
    if (detail::user_data_folder) return detail::user_data_folder;

    auto fs = user_data_filesystem();
    detail::user_data_folder = std::make_shared<User_Data_Folder>(*fs, env);
    return detail::user_data_folder;
}

bool file_utilities::open_url(const std::string& url) {
    bool result = false;

    #if defined(_WIN32)
        // Not using std::system("start ...") because it opens a console window
        result = detail::open_windows_path(url);
    #else
        std::string command;

        #if defined(__APPLE__)
            command = "open \"%\"";
        #else
            command = "xdg-open \"%\" >/dev/null 2>&1 &";
        #endif

        string_utilities::replace_all(command, "%", url);
        result = std::system(command.c_str()) == 0;
    #endif

    if (result) {
        LOGGER_I << "Opened URL " << url;
    } else {
        LOGGER_E << "Failed to open URL: " << url;
    }

    return result;
}
