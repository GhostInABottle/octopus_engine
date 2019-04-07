#ifndef H_XD_LUA
#define H_XD_LUA

#include "lua/types.hpp"
#include "lua/exceptions.hpp"
#include "lua/virtual_machine.hpp"
#include "lua/scheduler.hpp"

#ifndef LUABIND_CPLUSPLUS_LUA
extern "C"
{
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifndef LUABIND_CPLUSPLUS_LUA
}
#endif
#include <luabind/luabind.hpp>

#endif
