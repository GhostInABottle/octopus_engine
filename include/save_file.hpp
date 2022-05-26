#ifndef HPP_SAVE_FILE
#define HPP_SAVE_FILE

#include <iosfwd>
#include "../include/xd/lua.hpp"

class Save_File {
public:
    Save_File(sol::state& state, bool header_only = false, bool compact = true);
    Save_File(sol::state& state, const sol::table& data, std::optional<sol::table> header = std::nullopt, bool compact = true);
    sol::object& lua_data() { return data; }
    sol::object& header_data() { return header; }
    bool is_valid() const { return valid; }
    friend std::ostream& operator<<(std::ostream& stream, Save_File& save_file);
    friend std::istream& operator>>(std::istream& stream, Save_File& save_file);
private:
    sol::state& state;
    sol::table header;
    sol::table data;
    bool header_only;
    bool compact;
    bool valid;
};

std::ostream& operator<<(std::ostream& stream, Save_File& save_file);
std::istream& operator>>(std::istream& stream, Save_File& save_file);

#endif
