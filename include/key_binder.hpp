#ifndef HPP_KEY_BINDER
#define HPP_KEY_BINDER

#include <unordered_map>
#include <vector>
#include "xd/system/input.hpp"

class Game;

// Binds physical keys to virtual names and processes the keymap file
class Key_Binder {
public:
    Key_Binder(Game& game);
    // Bind a key by physical name to a virtual name
    void bind_key(const std::string& physical_name, const std::string& virtual_name);
    // Bind a physical key
    void bind_key(const xd::key& key, const std::string& virtual_name);
    // Bind some default values
    void bind_defaults();
    // Remove a virtual name and its bindings
    void remove_virtual_name(const std::string& virtual_name);
    // Remove binding for a key by physical name
    void unbind_key(const std::string& physical_name);
    // Remove binding for a key
    void unbind_key(const xd::key& key);
    // Process the keymap file and return whether it succeeded
    bool process_keymap_file();
    // Save the keymap file and return whether it succeeded
    bool save_keymap_file();
    // Get the filename of the keymap file
    std::string get_keymap_filename() const;
    // Get the physical name for a key
    std::string get_key_name(const xd::key& key) const {
        if (name_for_key.find(key) != name_for_key.end())
            return name_for_key.at(key);
        else
            return "";
    }
    // Get list of keys with the given physical name
    std::vector<xd::key> get_keys(const std::string& physical_name) const;
    // Get list of bound physical key names for a virtual name
    std::vector<std::string> get_bound_keys(const std::string& virtual_name) const {
        if (bound_keys.find(virtual_name) != bound_keys.end())
            return bound_keys.at(virtual_name);
        else
            return {};
    }
private:
    Game& game;
    bool changed_since_save;
    std::unordered_map<std::string, std::vector<xd::key>> keys_for_name;
    std::unordered_map<xd::key, std::string> name_for_key;
    std::unordered_map<std::string, std::vector<std::string>> bound_keys;
};

#endif