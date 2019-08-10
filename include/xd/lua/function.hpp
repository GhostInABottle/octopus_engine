#ifndef H_XD_LUA_FUNCTION
#define H_XD_LUA_FUNCTION

#ifndef LUABIND_CPLUSPLUS_LUA
extern "C"
{
#endif
#include <lua.h>
#ifndef LUABIND_CPLUSPLUS_LUA
}
#endif
#include <luabind/luabind.hpp>
#include <optional>


namespace xd
{
    namespace lua
    {
        template <typename T>
        struct function
        {
            function(const luabind::object& obj)
                : m_func(obj)
            {
            }

            function(const luabind::object& obj, const T& default_)
                : m_func(obj)
                , m_default(default_)
            {
            }

            luabind::object object()
            {
                return m_func;
            }

            template <typename... Args>
            T operator()(const Args&... args)
            {
                luabind::object ret = m_func(args...);
                if (m_default && luabind::type(ret) == LUA_TNIL)
                    return *m_default;
                return luabind::object_cast<T>(ret);
            }

        private:
            luabind::object m_func;
            std::optional<T> m_default;
        };

        template <>
        struct function<void>
        {
            function(const luabind::object& obj)
                : m_func(obj)
            {
            }

            luabind::object object()
            {
                return m_func;
            }

            template <typename... Args>
            void operator()(const Args&... args)
            {
                m_func(args...);
            }

        private:
            luabind::object m_func;
        };
    }
}

#endif
