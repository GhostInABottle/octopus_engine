#include "../include/key_binder.hpp"
#include "../include/game.hpp"
#include "../include/utility/string.hpp"
#include "../include/log.hpp"
#include "../include/configurations.hpp"
#include <string>
#include <fstream>

Key_Binder::Key_Binder(Game& game, bool gamepad_enabled) : game(game), gamepad_enabled(gamepad_enabled) {
    // Map key names to XD keys
    key_names["LEFT"] = { xd::KEY_LEFT };
    key_names["RIGHT"] = { xd::KEY_RIGHT };
    key_names["UP"] = { xd::KEY_UP };
    key_names["DOWN"] = { xd::KEY_DOWN };
    key_names["ENTER"] = { xd::KEY_ENTER };
    key_names["SPACE"] = { xd::KEY_SPACE };
    key_names["ESC"] = { xd::KEY_ESC };
    key_names["LEFT_CTRL"] = { xd::KEY_LCTRL };
    key_names["RIGHT_CTRL"] = { xd::KEY_RCTRL };
    key_names["CTRL"] = { xd::KEY_LCTRL, xd::KEY_RCTRL };
    key_names["LEFT_ALT"] = { xd::KEY_LALT };
    key_names["RIGHT_ALT"] = { xd::KEY_RALT };
    key_names["ALT"] = { xd::KEY_LALT, xd::KEY_RALT };
    key_names["LEFT_SHIFT"] = { xd::KEY_LSHIFT };
    key_names["RIGHT_SHIFT"] = { xd::KEY_RSHIFT };
    key_names["SHIFT"] = { xd::KEY_LSHIFT, xd::KEY_RSHIFT };
    // Assumes ASCII layout
    for (int i = xd::KEY_A.code; i <= xd::KEY_Z.code; ++i) {
        key_names[std::string(1, i)] = { xd::KEY(i) };
    }
    for (int i = xd::KEY_0.code; i <= xd::KEY_9.code; ++i) {
        key_names[std::string(1, i)] = { xd::KEY(i) };
    }
    if (gamepad_enabled) {
        key_names["GAMEPAD-A"] = { xd::GAMEPAD_BUTTON_A };
        key_names["GAMEPAD-B"] = { xd::GAMEPAD_BUTTON_B };
        key_names["GAMEPAD-X"] = { xd::GAMEPAD_BUTTON_X };
        key_names["GAMEPAD-Y"] = { xd::GAMEPAD_BUTTON_Y };
        key_names["GAMEPAD-LB"] = { xd::GAMEPAD_BUTTON_LEFT_BUMPER };
        key_names["GAMEPAD-RB"] = {xd::GAMEPAD_BUTTON_RIGHT_BUMPER };
        key_names["GAMEPAD-BACK"] = { xd::GAMEPAD_BUTTON_BACK };
        key_names["GAMEPAD-START"] = { xd::GAMEPAD_BUTTON_START };
        key_names["GAMEPAD-GUIDE"] = { xd::GAMEPAD_BUTTON_GUIDE };
        key_names["GAMEPAD-LT"] = { xd::GAMEPAD_BUTTON_LEFT_THUMB };
        key_names["GAMEPAD-RT"] = { xd::GAMEPAD_BUTTON_RIGHT_THUMB };
        key_names["GAMEPAD-UP"] = { xd::GAMEPAD_BUTTON_DPAD_UP };
        key_names["GAMEPAD-RIGHT"] = { xd::GAMEPAD_BUTTON_DPAD_RIGHT };
        key_names["GAMEPAD-DOWN"] = { xd::GAMEPAD_BUTTON_DPAD_DOWN };
        key_names["GAMEPAD-LEFT"] = { xd::GAMEPAD_BUTTON_DPAD_LEFT };
    }
}
void Key_Binder::bind_key(const std::string& physical_name, const std::string& virtual_name) {
    auto key = capitalize(physical_name);
    if (key_names.find(key) != key_names.end()) {
        for (auto& xd_key : key_names[key]) {
            game.bind_key(xd_key, virtual_name);
        }
    } else {
        LOGGER_W << "Physical key name " << physical_name <<
            " was not found while trying to bind key " << virtual_name;
    }
}

void Key_Binder::unbind_key(const std::string& physical_name) {
    auto key = capitalize(physical_name);
    if (key_names.find(key) != key_names.end()) {
        for (auto& xd_key : key_names[key]) {
            game.unbind_physical_key(xd_key);
        }
    } else {
        LOGGER_W << "Physical key name " << physical_name <<
            " was not found while trying unbind it";
    }
}

bool Key_Binder::process_keymap_file() {
    std::string filename = Configurations::get<std::string>("controls.mapping-file");
    std::ifstream input(filename);
    if (!input) {
        LOGGER_W << "Couldn't read key mapping file \"" << filename << "\", using default key mapping.";
        return false;
    }
    // Read keymap file and bind keys based on name
    std::string line;
    int counter = 0;
    while (std::getline(input, line))
    {
        ++counter;
        trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        auto parts = split(line, "=");
        if (parts.size() < 2) {
            LOGGER_W << "Error processing key mapping file \"" << filename <<
                " at line " << counter << ", missing = sign.";
            continue;
        }
        trim(parts[0]);
        auto keys = split(parts[1], ",");
        if (keys.empty())
            LOGGER_W << "Error processing key mapping file \"" << filename <<
            " at line " << counter << ", no keys specified.";
        for (auto key : keys) {
            trim(key);
            capitalize(key);
            if (key_names.find(key) != key_names.end()) {
                for (auto& xd_key : key_names[key]) {
                    game.bind_key(xd_key, parts[0]);
                }
            } else if (gamepad_enabled || key.find("GAMEPAD") == std::string::npos) {
                LOGGER_W << "Error processing key mapping file \"" << filename <<
                    " at line " << counter << ", key \"" << key << "\" not found.";
                continue;
            }
        }
    }
    return true;
}