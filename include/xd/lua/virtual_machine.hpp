#ifndef H_XD_LUA_VIRTUAL_MACHINE
#define H_XD_LUA_VIRTUAL_MACHINE

#include "types.hpp"
#include "exceptions.hpp"
#ifndef LUABIND_CPLUSPLUS_LUA
extern "C"
{
#endif
#include <lua.h>
#ifndef LUABIND_CPLUSPLUS_LUA
}
#endif
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <string>

namespace xd
{
    namespace lua
    {
        class virtual_machine
        {
        public:
            virtual_machine(const virtual_machine&) = delete;
            virtual_machine& operator=(const virtual_machine&) = delete;
            virtual_machine();
            virtual ~virtual_machine();

            lua_State *lua_state();
            function<void> load(const std::string& code);
            function<void> load_file(const std::string& filename);
            void exec(const std::string& code);
            void exec_file(const std::string& filename);

            void load_library(const std::string& module_name = "xd");

            luabind::object globals()
            {
                return luabind::globals(m_lua_state);
            }

            template <typename T>
            luabind::object globals(const T& key)
            {
                return luabind::globals(m_lua_state)[key];
            }

            luabind::object registry()
            {
                return luabind::registry(m_lua_state);
            }

            template <typename T>
            luabind::object registry(const T& key)
            {
                return luabind::registry(m_lua_state)[key];
            }

        private:
            lua_State *m_lua_state;
        };
    }
}

#endif
