#ifndef H_XD_LUA_TYPES
#define H_XD_LUA_TYPES

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
#include <optional>
#include <any>

namespace xd
{
    namespace lua
    {
        template <typename Class, typename Type, std::optional<Type> Class::*Ptr>
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
                    obj.*Ptr = std::nullopt;
                else
                    obj.*Ptr = luabind::object_cast<Type>(val);
            }
        };
    }
}

#endif
