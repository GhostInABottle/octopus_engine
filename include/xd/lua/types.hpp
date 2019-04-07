#ifndef H_XD_LUA_TYPES
#define H_XD_LUA_TYPES

#include "../types.hpp"
#include "function.hpp"
#ifndef LUABIND_CPLUSPLUS_LUA
extern "C"
{
#endif
#include <lua.h>
#ifndef LUABIND_CPLUSPLUS_LUA
}
#endif
#include <luabind/luabind.hpp>
#include <boost/optional.hpp>

namespace xd
{
    namespace lua
    {
        template <typename Class, typename Type, boost::optional<Type> Class::*Ptr>
        struct optional_property
        {
            static luabind::object getter(lua_State *vm, const Class& obj)
            {
                if (!(obj.*Ptr))
                    return luabind::object();
                else
                    return luabind::object(vm, *(obj.*Ptr));
            }

            static void setter(Class& obj, const luabind::object& val)
            {
                if (luabind::type(val) == LUA_TNIL)
                    obj.*Ptr = xd::any;
                else
                    obj.*Ptr = luabind::object_cast<Type>(val);
            }
        };
    }
}

#endif
