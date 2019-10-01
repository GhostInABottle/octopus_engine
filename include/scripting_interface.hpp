#ifndef HPP_SCRIPTING_INTERFACE
#define HPP_SCRIPTING_INTERFACE

#include <vector>
#include <memory>
#include <string>
#include "xd/lua.hpp"

class Game;
class Command;
class Command_Result;
class Choice_Result;

class Scripting_Interface {
public:
    Scripting_Interface(const Scripting_Interface&) = delete;
    Scripting_Interface& operator=(const Scripting_Interface&) = delete;
    explicit Scripting_Interface(Game& game);
    void update();
    void run_script(const std::string& script);
    void set_globals();
    xd::lua::scheduler& get_scheduler() { return scheduler; }
    std::unique_ptr<Command_Result> register_command(std::shared_ptr<Command> command);
    std::unique_ptr<Choice_Result> register_choice_command(std::shared_ptr<Command> command);
    sol::state& lua_state();
private:
    void setup_scripts();
    static Game* game;
    xd::lua::scheduler scheduler;
    std::vector<std::shared_ptr<Command>> commands;
};

#endif
