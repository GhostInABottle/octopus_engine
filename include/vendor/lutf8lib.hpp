#ifndef lutf8lib_hpp
#define lutf8lib_hpp

#define SOL_USING_CXX_LUA 1

struct lua_State;

int luaopen_utf8(lua_State* L);

#endif