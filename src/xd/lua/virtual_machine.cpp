#include "../../../include/xd/lua/virtual_machine.hpp"
#include "../../../include/vendor/lutf8lib.hpp"

xd::lua::virtual_machine::virtual_machine()
{
    // open some common libraries
    m_lua_state.open_libraries(sol::lib::base,
        sol::lib::package,
        sol::lib::coroutine,
        sol::lib::string,
        sol::lib::os,
        sol::lib::math,
        sol::lib::table,
        sol::lib::debug,
        sol::lib::io);
    luaopen_utf8(m_lua_state.lua_state());
}

