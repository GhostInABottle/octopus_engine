#ifndef H_XD_LUA_FUNCTION
#define H_XD_LUA_FUNCTION

#include "../config.hpp"
#include "config.hpp"
#ifndef LUABIND_CPLUSPLUS_LUA
extern "C"
{
#endif
#include <lua.h>
#ifndef LUABIND_CPLUSPLUS_LUA
}
#endif
#include <luabind/luabind.hpp>
#include <boost/config.hpp>
#include <boost/optional.hpp>

#ifdef BOOST_NO_VARIADIC_TEMPLATES
#include <boost/preprocessor/iteration/iterate.hpp>
#endif

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

#ifndef BOOST_NO_VARIADIC_TEMPLATES
            template <typename... Args>
            T operator()(const Args&... args)
            {
                luabind::object ret = m_func(args...);
                if (m_default && luabind::type(ret) == LUA_TNIL)
                    return *m_default;
                return luabind::object_cast<T>(ret);
            }
#else
            // generate overloads for operator()
            // maximum of XD_MAX_ARITY parameters supported
            #define BOOST_PP_ITERATION_PARAMS_1 (4, (0, XD_MAX_ARITY, "../include/xd/lua/detail/iterate_function_call.hpp", 0))
            #include BOOST_PP_ITERATE()
#endif

        private:
            luabind::object m_func;
            boost::optional<T> m_default;
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

#ifndef BOOST_NO_VARIADIC_TEMPLATES
            template <typename... Args>
            void operator()(const Args&... args)
            {
                m_func(args...);
            }
#else
            // generate overloads for operator()
            // maximum of XD_MAX_ARITY parameters supported
            #define BOOST_PP_ITERATION_PARAMS_1 (4, (0, XD_MAX_ARITY, "../include/xd/lua/detail/iterate_function_call.hpp", 1))
            #include BOOST_PP_ITERATE()
#endif
        private:
            luabind::object m_func;
        };
    }
}

#endif
