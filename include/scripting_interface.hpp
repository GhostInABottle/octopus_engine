#ifndef HPP_SCRIPTING_INTERFACE
#define HPP_SCRIPTING_INTERFACE

#include <vector>
#include <memory>
#include <string>
#include <xd/lua.hpp>

class Game;
class Command;
class Command_Result;
class Choice_Result;

class Scripting_Interface {
public:
    Scripting_Interface(Game& game) : scheduler(vm) {
        if (!Scripting_Interface::game) {
            Scripting_Interface::game = &game;
            setup_scripts();
        }
    }
    void update();
    void run_script(const std::string& script);
    void set_globals();
    xd::lua::scheduler& get_scheduler() { return scheduler; }
    Command_Result* register_command(std::shared_ptr<Command> command);
    Choice_Result* register_choice_command(std::shared_ptr<Command> command);
    lua_State* lua_state() {
        return vm.lua_state();
    }
private:
    static void setup_scripts();
    static Game* game;
    static xd::lua::virtual_machine vm;
    xd::lua::scheduler scheduler;
    std::vector<std::shared_ptr<Command>> commands;
};

#endif
