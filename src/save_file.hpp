#ifndef HPP_SAVE_FILE
#define HPP_SAVE_FILE

#include <memory>
#include <iosfwd>
#include "xd/vendor/sol/forward.hpp"

class Save_File {
public:
    Save_File(sol::state& state, bool header_only = false, bool compact = true);
    Save_File(sol::state& state, const sol::table& data, const sol::table* header = nullptr, bool compact = true);
    ~Save_File();
    sol::object& lua_data();
    sol::object& header_data();
    bool is_valid() const;
    friend std::ostream& operator<<(std::ostream& stream, Save_File& save_file);
    friend std::istream& operator>>(std::istream& stream, Save_File& save_file);
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

std::ostream& operator<<(std::ostream& stream, Save_File& save_file);
std::istream& operator>>(std::istream& stream, Save_File& save_file);

#endif
