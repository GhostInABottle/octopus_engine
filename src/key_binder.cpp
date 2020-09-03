#include "../include/key_binder.hpp"
#include "../include/game.hpp"
#include "../include/utility/string.hpp"
#include "../include/utility/file.hpp"
#include "../include/log.hpp"
#include "../include/configurations.hpp"
#include <string>
#include <fstream>
#include <algorithm>

Key_Binder::Key_Binder(Game& game)
        : game(game), changed_since_save(false) {
    // Map key names to XD keys
    keys_for_name["LEFT"] = { xd::KEY_LEFT };
    keys_for_name["RIGHT"] = { xd::KEY_RIGHT };
    keys_for_name["UP"] = { xd::KEY_UP };
    keys_for_name["DOWN"] = { xd::KEY_DOWN };
    keys_for_name["ENTER"] = { xd::KEY_ENTER };
    keys_for_name["SPACE"] = { xd::KEY_SPACE };
    keys_for_name["ESC"] = { xd::KEY_ESC };
    keys_for_name["LEFT_CTRL"] = { xd::KEY_LCTRL };
    keys_for_name["RIGHT_CTRL"] = { xd::KEY_RCTRL };
    keys_for_name["CTRL"] = { xd::KEY_LCTRL, xd::KEY_RCTRL };
    keys_for_name["LEFT_ALT"] = { xd::KEY_LALT };
    keys_for_name["RIGHT_ALT"] = { xd::KEY_RALT };
    keys_for_name["ALT"] = { xd::KEY_LALT, xd::KEY_RALT };
    keys_for_name["LEFT_SHIFT"] = { xd::KEY_LSHIFT };
    keys_for_name["RIGHT_SHIFT"] = { xd::KEY_RSHIFT };
    keys_for_name["SHIFT"] = { xd::KEY_LSHIFT, xd::KEY_RSHIFT };
    // Assumes ASCII layout
    for (int i = xd::KEY_A.code; i <= xd::KEY_Z.code; ++i) {
        keys_for_name[std::string(1, i)] = { xd::KEY(i) };
    }
    for (int i = xd::KEY_0.code; i <= xd::KEY_9.code; ++i) {
        keys_for_name[std::string(1, i)] = { xd::KEY(i) };
    }
    keys_for_name["GAMEPAD-A"] = { xd::GAMEPAD_BUTTON_A };
    keys_for_name["GAMEPAD-B"] = { xd::GAMEPAD_BUTTON_B };
    keys_for_name["GAMEPAD-X"] = { xd::GAMEPAD_BUTTON_X };
    keys_for_name["GAMEPAD-Y"] = { xd::GAMEPAD_BUTTON_Y };
    keys_for_name["GAMEPAD-LB"] = { xd::GAMEPAD_BUTTON_LEFT_BUMPER };
    keys_for_name["GAMEPAD-RB"] = { xd::GAMEPAD_BUTTON_RIGHT_BUMPER };
    keys_for_name["GAMEPAD-BACK"] = { xd::GAMEPAD_BUTTON_BACK };
    keys_for_name["GAMEPAD-START"] = { xd::GAMEPAD_BUTTON_START };
    keys_for_name["GAMEPAD-GUIDE"] = { xd::GAMEPAD_BUTTON_GUIDE };
    keys_for_name["GAMEPAD-LT"] = { xd::GAMEPAD_BUTTON_LEFT_THUMB };
    keys_for_name["GAMEPAD-RT"] = { xd::GAMEPAD_BUTTON_RIGHT_THUMB };
    keys_for_name["GAMEPAD-UP"] = { xd::GAMEPAD_BUTTON_DPAD_UP };
    keys_for_name["GAMEPAD-RIGHT"] = { xd::GAMEPAD_BUTTON_DPAD_RIGHT };
    keys_for_name["GAMEPAD-DOWN"] = { xd::GAMEPAD_BUTTON_DPAD_DOWN };
    keys_for_name["GAMEPAD-LEFT"] = { xd::GAMEPAD_BUTTON_DPAD_LEFT };
    // Map the other side of the relationship
    for (auto& [name, keys] : keys_for_name) {
        for (auto& key : keys) {
            name_for_key[key] = name;
        }
    }
}
void Key_Binder::bind_key(const std::string& physical_name, const std::string& virtual_name) {
    auto key = string_utilities::capitalize(physical_name);
    if (keys_for_name.find(key) != keys_for_name.end()) {
        for (auto& xd_key : keys_for_name[key]) {
            game.bind_key(xd_key, virtual_name);
        }
        auto& values = bound_keys[virtual_name];
        if (std::find(std::begin(values), std::end(values), key) == std::end(values)) {
            values.push_back(key);
            changed_since_save = true;
        }
    } else {
        LOGGER_W << "Physical key name " << physical_name <<
            " was not found while trying to bind key " << virtual_name;
    }
}

void Key_Binder::bind_key(const xd::key& key, const std::string& virtual_name) {
    if (name_for_key.find(key) != name_for_key.end()) {
        bind_key(name_for_key[key], virtual_name);
    } else {
        LOGGER_W << "Name of key with code " << key.code <<
            " was not found while trying to bind key " << virtual_name;
    }
}

void Key_Binder::bind_defaults() {
    // Default mapping
    bound_keys.clear();
    bind_key(xd::KEY_ESC, "pause");
    bind_key(xd::KEY_LEFT, "left");
    bind_key(xd::KEY_A, "left");
    bind_key(xd::KEY_RIGHT, "right");
    bind_key(xd::KEY_D, "right");
    bind_key(xd::KEY_UP, "up");
    bind_key(xd::KEY_W, "up");
    bind_key(xd::KEY_DOWN, "down");
    bind_key(xd::KEY_S, "down");
    bind_key(xd::KEY_ENTER, "a");
    bind_key(xd::KEY_SPACE, "a");
    bind_key(xd::KEY_Z, "a");
    bind_key(xd::KEY_J, "a");
    bind_key(xd::KEY_X, "b");
    bind_key(xd::KEY_K, "b");
    bind_key(xd::KEY_C, "x");
    bind_key(xd::KEY_L, "x");
    bind_key(xd::KEY_V, "y");
    bind_key(xd::KEY_I, "y");
    bind_key(xd::GAMEPAD_BUTTON_DPAD_UP, "up");
    bind_key(xd::GAMEPAD_BUTTON_DPAD_DOWN, "down");
    bind_key(xd::GAMEPAD_BUTTON_DPAD_LEFT, "left");
    bind_key(xd::GAMEPAD_BUTTON_DPAD_RIGHT, "right");
    bind_key(xd::GAMEPAD_BUTTON_A, "a");
    bind_key(xd::GAMEPAD_BUTTON_B, "b");
    bind_key(xd::GAMEPAD_BUTTON_X, "x");
    bind_key(xd::GAMEPAD_BUTTON_Y, "y");
    bind_key(xd::GAMEPAD_BUTTON_START, "pause");
}

void Key_Binder::remove_virtual_name(const std::string& virtual_name) {
    bound_keys.erase(virtual_name);
    changed_since_save = true;
}

void Key_Binder::unbind_key(const std::string& physical_name) {
    auto key = string_utilities::capitalize(physical_name);
    if (keys_for_name.find(key) != keys_for_name.end()) {
        for (auto& xd_key : keys_for_name[key]) {
            game.unbind_physical_key(xd_key);
        }
        for (auto& [key_name, values] : bound_keys) {
            values.erase(std::remove(values.begin(), values.end(), key), values.end());
        }
        changed_since_save = true;
    } else {
        LOGGER_W << "Physical key name " << physical_name <<
            " was not found while trying unbind it";
    }
}

void Key_Binder::unbind_key(const xd::key& key) {
    if (name_for_key.find(key) != name_for_key.end()) {
        unbind_key(name_for_key[key]);
    } else {
        LOGGER_W << "Name of key with code" << key.code <<
            " was not found while trying unbind it";
    }
}

bool Key_Binder::process_keymap_file() {
    std::string filename = get_keymap_filename();
    if (!file_utilities::file_exists(filename)) {
        filename = Configurations::get<std::string>("controls.mapping-file");
    }
    std::ifstream input(filename);
    if (!input) {
        LOGGER_W << "Couldn't read key mapping file \"" << filename << "\", using default key mapping.";
        return false;
    }
    LOGGER_I << "Processing keymap file " << filename;
    bound_keys.clear();
    // Read keymap file and bind keys based on name
    std::string line;
    int counter = 0;
    while (std::getline(input, line))
    {
        ++counter;
        string_utilities::trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        auto parts = string_utilities::split(line, "=");
        if (parts.size() < 2) {
            LOGGER_W << "Error processing key mapping file \"" << filename <<
                " at line " << counter << ", missing = sign.";
            continue;
        }
        auto virtual_name = parts[0];
        string_utilities::trim(virtual_name);
        if (virtual_name.empty())
            LOGGER_E << "Error processing key mapping file \"" << filename <<
            " at line " << counter << ", virtual name is missing.";
        auto keys = string_utilities::split(parts[1], ",");
        if (keys.empty())
            LOGGER_W << "Error processing key mapping file \"" << filename <<
            " at line " << counter << ", no keys specified.";
        for (auto key : keys) {
            string_utilities::trim(key);
            string_utilities::capitalize(key);
            if (keys_for_name.find(key) != keys_for_name.end()) {
                bind_key(key, virtual_name);
            } else {
                LOGGER_W << "Error processing key mapping file \"" << filename <<
                    " at line " << counter << ", key \"" << key << "\" not found.";
            }
        }
    }
    changed_since_save = true;
    return true;
}

bool Key_Binder::save_keymap_file() {
    if (!changed_since_save || !Configurations::get<bool>("debug.update-config-files")) return true;
    auto filename = get_keymap_filename();
    std::ofstream output(filename);
    if (!output) {
        LOGGER_E << "Unable to open keymap file " << filename << " for writing";
        return false;
    }
    LOGGER_I << "Saving keymap file " << filename;
    output << "# Logical name = key1, key2, gamepad-key1, etc.\n"
        "# Special keys allowed: enter, space, esc, ctrl, alt, and directions\n";
    if (!output) {
        LOGGER_E << "Error writing opening comment to keymap file";
        return false;
    }
    for (auto& [virtual_name, physical_names] : bound_keys) {
        auto values = string_utilities::join(physical_names, ", ");
        if (values.empty()) continue;
        output << virtual_name << " = " <<values << "\n";
        if (!output) {
            LOGGER_E << "Error writing key mapping '" << virtual_name
                << "=" << values << "' to keymap file " << filename;
            return false;
        }
    }
    LOGGER_I << "Finished saving keymap file " << filename;
    changed_since_save = false;
    return true;
}

std::string Key_Binder::get_keymap_filename() const {
    std::string filename = Configurations::get<std::string>("controls.mapping-file");
    if (file_utilities::is_absolute_path(filename)) return filename;
    auto data_directory = file_utilities::get_data_directory();
    if (data_directory.empty()) return filename;
    return data_directory + file_utilities::get_filename_component(filename);
}

std::vector<xd::key> Key_Binder::get_keys(const std::string& physical_name) const {
    auto key_name = string_utilities::capitalize(physical_name);
    if (keys_for_name.find(key_name) != keys_for_name.end())
        return keys_for_name.at(key_name);
    else
        return {};
}