#include "key_binder.hpp"
#include "game.hpp"
#include "utility/string.hpp"
#include "log.hpp"
#include "configurations.hpp"
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
    keys_for_name["APOSTROPHE"] = { xd::KEY_APOSTROPHE };
    keys_for_name["BACKSLASH"] = { xd::KEY_BACKSLASH };
    keys_for_name["BACKSPACE"] = { xd::KEY_BACKSPACE };
    keys_for_name["CAPSLOCK"] = { xd::KEY_CAPSLOCK };
    keys_for_name["COMMA"] = { xd::KEY_COMMA };
    keys_for_name["DELETE"] = { xd::KEY_DELETE };
    keys_for_name["END"] = { xd::KEY_END };
    keys_for_name["EQUAL"] = { xd::KEY_EQUAL };
    keys_for_name["F1"] = { xd::KEY_F1 };
    keys_for_name["F2"] = { xd::KEY_F2 };
    keys_for_name["F3"] = { xd::KEY_F3 };
    keys_for_name["F4"] = { xd::KEY_F4 };
    keys_for_name["F5"] = { xd::KEY_F5 };
    keys_for_name["F6"] = { xd::KEY_F6 };
    keys_for_name["F7"] = { xd::KEY_F7 };
    keys_for_name["F8"] = { xd::KEY_F8 };
    keys_for_name["F9"] = { xd::KEY_F9 };
    keys_for_name["F10"] = { xd::KEY_F10 };
    keys_for_name["F11"] = { xd::KEY_F11 };
    keys_for_name["F12"] = { xd::KEY_F12 };
    keys_for_name["GRAVEACCENT"] = { xd::KEY_GRAVEACCENT };
    keys_for_name["HOME"] = { xd::KEY_HOME };
    keys_for_name["INSERT"] = { xd::KEY_INSERT };
    keys_for_name["NUMPAD0"] = { xd::KEY_NUMPAD0 };
    keys_for_name["NUMPAD1"] = { xd::KEY_NUMPAD1 };
    keys_for_name["NUMPAD2"] = { xd::KEY_NUMPAD2 };
    keys_for_name["NUMPAD3"] = { xd::KEY_NUMPAD3 };
    keys_for_name["NUMPAD4"] = { xd::KEY_NUMPAD4 };
    keys_for_name["NUMPAD5"] = { xd::KEY_NUMPAD5 };
    keys_for_name["NUMPAD6"] = { xd::KEY_NUMPAD6 };
    keys_for_name["NUMPAD7"] = { xd::KEY_NUMPAD7 };
    keys_for_name["NUMPAD8"] = { xd::KEY_NUMPAD8 };
    keys_for_name["NUMPAD9"] = { xd::KEY_NUMPAD9 };
    keys_for_name["NUMPADPLUS"] = { xd::KEY_NUMPADPLUS };
    keys_for_name["NUMPADDECIMAL"] = { xd::KEY_NUMPADDECIMAL };
    keys_for_name["NUMPADDIVIDE"] = { xd::KEY_NUMPADDIVIDE };
    keys_for_name["NUMPADENTER"] = { xd::KEY_NUMPADENTER };
    keys_for_name["NUMPADEQUAL"] = { xd::KEY_NUMPADEQUAL };
    keys_for_name["NUMPADTIMES"] = { xd::KEY_NUMPADTIMES };
    keys_for_name["NUMPADMINUS"] = { xd::KEY_NUMPADMINUS };
    keys_for_name["LBRACKET"] = { xd::KEY_LBRACKET };
    keys_for_name["RBRACKET"] = { xd::KEY_RBRACKET };
    keys_for_name["LSUPER"] = { xd::KEY_LSUPER };
    keys_for_name["RSUPER"] = { xd::KEY_RSUPER };
    keys_for_name["MENU"] = { xd::KEY_MENU };
    keys_for_name["MINUS"] = { xd::KEY_MINUS };
    keys_for_name["NUMLOCK"] = { xd::KEY_NUMLOCK };
    keys_for_name["PAGEDOWN"] = { xd::KEY_PAGEDOWN };
    keys_for_name["PAGEUP"] = { xd::KEY_PAGEUP };
    keys_for_name["PAUSE"] = { xd::KEY_PAUSE };
    keys_for_name["PERIOD"] = { xd::KEY_PERIOD };
    keys_for_name["PRTSCN"] = { xd::KEY_PRTSCN };
    keys_for_name["SCROLLLOCK"] = { xd::KEY_SCROLLLOCK };
    keys_for_name["SEMICOLON"] = { xd::KEY_SEMICOLON };
    keys_for_name["SLASH"] = { xd::KEY_SLASH };
    keys_for_name["TAB"] = { xd::KEY_TAB };
    // Assumes ASCII layout
    for (int i = xd::KEY_A.code; i <= xd::KEY_Z.code; ++i) {
        std::string name(1, static_cast<std::string::value_type>(i));
        keys_for_name[name] = { xd::KEY(i) };
    }
    for (int i = xd::KEY_0.code; i <= xd::KEY_9.code; ++i) {
        std::string name(1, static_cast<std::string::value_type>(i));
        keys_for_name[name] = { xd::KEY(i) };
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
    keys_for_name["GAMEPAD-LSB"] = { xd::GAMEPAD_BUTTON_LEFT_THUMB };
    keys_for_name["GAMEPAD-RSB"] = { xd::GAMEPAD_BUTTON_RIGHT_THUMB };
    keys_for_name["GAMEPAD-UP"] = { xd::GAMEPAD_BUTTON_DPAD_UP };
    keys_for_name["GAMEPAD-RIGHT"] = { xd::GAMEPAD_BUTTON_DPAD_RIGHT };
    keys_for_name["GAMEPAD-DOWN"] = { xd::GAMEPAD_BUTTON_DPAD_DOWN };
    keys_for_name["GAMEPAD-LEFT"] = { xd::GAMEPAD_BUTTON_DPAD_LEFT };
    keys_for_name["GAMEPAD-LT"] = { xd::GAMEPAD_BUTTON_LEFT_TRIGGER };
    keys_for_name["GAMEPAD-RT"] = { xd::GAMEPAD_BUTTON_RIGHT_TRIGGER };
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
        auto is_new_key = std::find(std::begin(values), std::end(values), key) == std::end(values);
        if (is_new_key) {
            values.push_back(key);
            changed_since_save = true;
        }
    } else {
        LOGGER_E << "Physical key name " << physical_name <<
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
        LOGGER_W << "Name of key with code " << key.code <<
            " was not found while trying unbind it";
    }
}

bool Key_Binder::process_keymap_file(std::istream& stream) {
    // Read keymap file and bind keys based on name
    std::string line;
    int counter = 0;
    while (std::getline(stream, line)) {
        ++counter;
        string_utilities::trim(line);
        if (line.empty() || line[0] == '#') continue;

        auto parts = string_utilities::split(line, "=");
        if (parts.size() < 2) {
            LOGGER_W << "Error processing key mapping file at line "
                << counter << ", missing = sign.";
            continue;
        }

        std::string virtual_name{parts[0]};
        string_utilities::trim(virtual_name);
        if (virtual_name.empty()) {
            LOGGER_E << "Error processing key mapping file  at line "
                << counter << ", virtual name is missing.";
        }

        auto keys = string_utilities::split(parts[1], ",");
        if (keys.empty()) {
            LOGGER_W << "Error processing key mapping file  at line "
                << counter << ", no keys specified.";
        }

        // Overwrite any existing bindings
        game.unbind_virtual_key(virtual_name);

        for (std::string key : keys) {
            string_utilities::trim(key);
            string_utilities::capitalize(key);
            if (keys_for_name.find(key) != keys_for_name.end()) {
                bind_key(key, virtual_name);
            } else {
                LOGGER_W << "Error processing key mapping file  at line "
                    << counter << ", key \"" << key << "\" not found.";
            }
        }
    }

    changed_since_save = true;
    return true;
}

bool Key_Binder::can_save() const {
    return changed_since_save && Configurations::get<bool>("debug.update-config-files");
}

bool Key_Binder::save_keymap_file(std::ostream& stream) {
    stream << "# Logical name = key1, key2, gamepad-key1, etc.\n"
        "# Special keys allowed: enter, space, esc, ctrl, alt, and directions\n";

    if (!stream) {
        LOGGER_E << "Error writing opening comment to keymap file";
        return false;
    }

    for (auto& [virtual_name, physical_names] : bound_keys) {
        auto values = string_utilities::join(physical_names, ", ");
        if (values.empty()) continue;

        stream << virtual_name << " = " <<values << "\n";
        if (!stream) {
            LOGGER_E << "Error writing key mapping '" << virtual_name
                << "=" << values << "' to keymap file";
            return false;
        }
    }

    changed_since_save = false;
    return true;
}

std::vector<xd::key> Key_Binder::get_keys(const std::string& physical_name) const {
    auto key_name = string_utilities::capitalize(physical_name);
    if (keys_for_name.find(key_name) != keys_for_name.end())
        return keys_for_name.at(key_name);
    else
        return {};
}
