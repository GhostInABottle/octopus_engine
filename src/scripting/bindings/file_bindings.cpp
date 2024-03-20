#include "../../../include/filesystem/path_info.hpp"
#include "../../../include/filesystem/readable_filesystem.hpp"
#include "../../../include/filesystem/user_data_folder.hpp"
#include "../../../include/filesystem/writable_filesystem.hpp"
#include "../../../include/game.hpp"
#include "../../../include/save_file.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <ctime>
#include <memory>
#include <optional>
#include <tuple>

void bind_file_types(sol::state& lua, Game& game) {
    // Calendar time
    auto tm_type = lua.new_usertype<std::tm>("Calendar_Time");
    tm_type["second"] = sol::readonly(&std::tm::tm_sec);
    tm_type["minute"] = sol::readonly(&std::tm::tm_min);
    tm_type["hour"] = sol::readonly(&std::tm::tm_hour);
    tm_type["month_day"] = sol::readonly(&std::tm::tm_mday);
    tm_type["month"] = sol::readonly(&std::tm::tm_mon);
    tm_type["year"] = sol::property([](std::tm& val) { return val.tm_year + 1900; });
    tm_type["week_day"] = sol::readonly(&std::tm::tm_wday);
    tm_type["year_day"] = sol::readonly(&std::tm::tm_yday);
    tm_type["is_dst"] = sol::property([](std::tm& val) {
        return val.tm_isdst < 0 ? std::nullopt : std::optional<bool>{ val.tm_isdst > 0 };
    });

    // Path info
    auto info_type = lua.new_usertype<Path_Info>("Path_Info");
    info_type["name"] = sol::readonly(&Path_Info::name);
    info_type["is_regular"] = sol::readonly(&Path_Info::is_regular);
    info_type["is_directory"] = sol::readonly(&Path_Info::is_directory);
    info_type["timestamp"] = sol::readonly(&Path_Info::timestamp);
    info_type["calendar_time"] = sol::readonly(&Path_Info::calendar_time);

    // Readable filesystem
    auto readable_fs_type = lua.new_usertype<Readable_Filesystem>("Readable_Filesystem");
    readable_fs_type["exists"] = &Readable_Filesystem::exists;
    readable_fs_type["is_regular_file"] = &Readable_Filesystem::is_regular_file;
    readable_fs_type["is_directory"] = &Readable_Filesystem::is_directory;
    readable_fs_type["list_directory"] = &Readable_Filesystem::directory_content_names;
    readable_fs_type["list_detailed_directory"] = &Readable_Filesystem::directory_content_details;
    readable_fs_type["is_absolute"] = &Readable_Filesystem::is_absolute_path;
    readable_fs_type["get_basename"] = &Readable_Filesystem::get_filename_component;
    readable_fs_type["get_stem"] = &Readable_Filesystem::get_stem_component;
    readable_fs_type["last_write_time"] = &Readable_Filesystem::last_write_time;

    // Writable filesystem
    auto writable_fs_type = lua.new_usertype<Writable_Filesystem>("Writable_Filesystem");
    writable_fs_type["exists"] = &Writable_Filesystem::exists;
    writable_fs_type["is_regular_file"] = &Writable_Filesystem::is_regular_file;
    writable_fs_type["is_directory"] = &Writable_Filesystem::is_directory;
    writable_fs_type["list_directory"] = &Writable_Filesystem::directory_content_names;
    writable_fs_type["list_detailed_directory"] = &Writable_Filesystem::directory_content_details;
    writable_fs_type["is_absolute"] = &Writable_Filesystem::is_absolute_path;
    writable_fs_type["get_basename"] = &Writable_Filesystem::get_filename_component;
    writable_fs_type["get_stem"] = &Writable_Filesystem::get_stem_component;
    writable_fs_type["last_write_time"] = &Writable_Filesystem::last_write_time;
    writable_fs_type["copy"] = &Writable_Filesystem::copy;
    writable_fs_type["remove"] = &Writable_Filesystem::remove;
    writable_fs_type["rename"] = &Writable_Filesystem::rename;

    // User Data Folder
    auto user_data_type = lua.new_usertype<User_Data_Folder>("User_Data_Folder");
    user_data_type["base_path"] = sol::property(&User_Data_Folder::get_base_path);
    user_data_type["game_path"] = sol::property(&User_Data_Folder::get_game_path);
    user_data_type["user_path"] = sol::property(&User_Data_Folder::get_user_path);
    user_data_type["version_path"] = sol::property(&User_Data_Folder::get_version_path);

    user_data_type["save"] = [&lua](User_Data_Folder* folder, const std::string& filename, sol::table obj,
            std::optional<sol::table> header, std::optional<bool> compact) {
        Save_File file(lua, obj, header ? &header.value() : nullptr, compact.value_or(true));
        return folder->save(filename, file);
    };
    user_data_type["load"] = [&lua](User_Data_Folder* folder, const std::string& filename, std::optional<bool> compact) {
        Save_File file(lua, false, compact.value_or(true));
        if (folder->load(filename, file))
            return std::make_tuple(file.lua_data(), file.header_data());
        else
            return std::make_tuple(sol::object(sol::lua_nil), sol::object(sol::lua_nil));
    };
    user_data_type["load_header"] = [&lua](User_Data_Folder* folder, const std::string& filename, std::optional<bool> compact) {
        Save_File file(lua, true, compact.value_or(true));
        if (folder->load(filename, file))
            return file.header_data();
        else
            return sol::object(sol::lua_nil);
    };

    user_data_type["save_config_file"] = [](User_Data_Folder& folder) {
        return folder.try_to_save_config();
    };

    user_data_type["save_keymap_file"] = [&game](User_Data_Folder& folder) {
        return folder.save_keymap_file(game.get_key_binder());
    };
}
