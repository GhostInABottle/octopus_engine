#ifndef HPP_SAVE_FILE
#define HPP_SAVE_FILE

#include <iosfwd>
#include <luabind/object.hpp>
#include <luabind/luabind.hpp>

class Save_File {
public:
    Save_File(lua_State* state, luabind::object data);
    luabind::object& lua_data() { return data; }
    friend std::ostream& operator<<(std::ostream& stream, const Save_File& save_file);
    friend std::istream& operator>>(std::istream& stream, Save_File& save_file);
private:
    lua_State* state;
    luabind::object data;
};

std::ostream& operator<<(std::ostream& stream, const Save_File& save_file);
std::istream& operator>>(std::istream& stream, Save_File& save_file);

#endif
