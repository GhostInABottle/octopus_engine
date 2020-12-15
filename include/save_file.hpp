#ifndef HPP_SAVE_FILE
#define HPP_SAVE_FILE

#include <iosfwd>
#include "../include/xd/lua.hpp"

class Save_File {
public:
    Save_File(sol::state& state);
    Save_File(sol::state& state, const sol::table& data);
    sol::object& lua_data() noexcept { return data; }
    bool is_valid() const noexcept { return valid; }
    friend std::ostream& operator<<(std::ostream& stream, Save_File& save_file);
    friend std::istream& operator>>(std::istream& stream, Save_File& save_file);
private:
    sol::state& state;
    sol::table data;
    bool valid;
};

std::ostream& operator<<(std::ostream& stream, Save_File& save_file);
std::istream& operator>>(std::istream& stream, Save_File& save_file);

#endif
