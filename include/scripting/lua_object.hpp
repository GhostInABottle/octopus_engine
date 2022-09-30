#ifndef HPP_LUA_OBJECT
#define HPP_LUA_OBJECT

#include <string>
#include <memory>
#include "../xd/vendor/sol/forward.hpp"

class Lua_Object {
public:
    Lua_Object();
    virtual ~Lua_Object();
    void set_lua_property(const std::string& name, sol::stack_object value);
    sol::main_object get_lua_property(const std::string& name);
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif