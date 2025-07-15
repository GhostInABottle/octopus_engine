#include "lua_object.hpp"
#include "../xd/vendor/sol/sol.hpp"
#include <unordered_map>

struct Lua_Object::Impl {
    std::unordered_map<std::string, sol::main_object> table;
};

Lua_Object::Lua_Object() : pimpl(std::make_unique<Impl>()) {}

Lua_Object::~Lua_Object() {}

void Lua_Object::set_lua_property(const std::string& name, sol::stack_object value) {
    pimpl->table[name] = sol::main_object(value);
}

sol::main_object Lua_Object::get_lua_property(const std::string& name) {
    auto it = pimpl->table.find(name);
    return it == pimpl->table.end() ? sol::lua_nil : it->second;
}
