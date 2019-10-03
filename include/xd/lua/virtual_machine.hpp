#ifndef H_XD_LUA_VIRTUAL_MACHINE
#define H_XD_LUA_VIRTUAL_MACHINE

#include "exceptions.hpp"
#include "sol.hpp"
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

            sol::state& lua_state() { return m_lua_state; }

            sol::table globals() const
            {
                return m_lua_state.globals();
            }

            template <typename T>
            sol::object globals(const T& key)
            {
                return m_lua_state.globals()[key];
            }

            sol::table registry()
            {
                return m_lua_state.registry();
            }

            template <typename T>
            sol::object registry(const T& key)
            {
                return m_lua_state.registry()[key];
            }

        private:
            sol::state m_lua_state;
        };
    }
}

#endif
