#ifndef HPP_KEY_BINDER
#define HPP_KEY_BINDER

#include <unordered_map>
#include <vector>
#include "xd/system/input.hpp"

class Game;

class Key_Binder {
public:
    Key_Binder(Game& game, bool gamepad_enabled);
    void bind_key(const std::string& physical_name, const std::string& virtual_name);
    void remove_virtual_name(const std::string& virtual_name);
    void unbind_key(const std::string& physical_name);
    bool process_keymap_file();
    bool save_keymap_file();
    std::string get_keymap_filename() const;
private:
    Game& game;
    bool gamepad_enabled;
    bool changed_since_save;
    std::unordered_map<std::string, std::vector<xd::key>> key_names;
    std::unordered_map<std::string, std::vector<std::string>> bound_keys;
};

#endif