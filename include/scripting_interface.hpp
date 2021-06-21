#ifndef HPP_SCRIPTING_INTERFACE
#define HPP_SCRIPTING_INTERFACE

#include <vector>
#include <memory>
#include <string>
#include <utility>
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
    void schedule_code(const std::string& script, const std::string& context = "");
    void schedule_file(const std::string& filename, const std::string& context = "");
    void schedule_function(const sol::protected_function& function, const std::string& context = "");
    template <typename T>
    T call(const std::string& script) {
        return lua_state().script(script).get<T>();
    }
    void set_globals();
    xd::lua::scheduler& get_scheduler() noexcept { return scheduler; }
    template<typename T, typename ... Args>
    std::unique_ptr<Command_Result> register_command(Args&& ... args) {
        auto command = std::make_shared<T>(std::forward<Args>(args)...);
        commands.push_back(command);
        return std::make_unique<Command_Result>(command);
    }
    template<typename T, typename ... Args>
    std::unique_ptr<Choice_Result> register_choice_command(Args&& ... args) {
        auto command = std::make_shared<T>(std::forward<Args>(args)...);
        commands.push_back(command);
        return std::make_unique<Choice_Result>(command);
    }
    sol::state& lua_state();
private:
    void setup_scripts();
    static Game* game;
    xd::lua::scheduler scheduler;
    std::vector<std::shared_ptr<Command>> commands;
};

#endif
