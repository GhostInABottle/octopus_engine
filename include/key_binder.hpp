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
    void unbind_key(const std::string& physical_name);
    bool process_keymap_file();
private:
    Game& game;
    bool gamepad_enabled;
    std::unordered_map<std::string, std::vector<xd::key>> key_names;
};

#endif