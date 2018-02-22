#include "../include/save_file.hpp"
#include <iostream>
#include <luabind/luabind.hpp>
#include <string>

namespace detail {
    template<typename T>
    void write(std::ostream& stream, const T& value) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }
    void write(std::ostream& stream, const std::string& value){
        write(stream, value.length());
        stream.write(value.c_str(), value.length());
    }
    template<typename T>
    void read(std::istream& stream, T& value){
        stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    }
    void read(std::istream& stream, std::string& value){
        std::string::size_type length;
        read(stream, length);
        value = std::string(length, ' ');
        stream.read(&value[0], length);
    }
    bool valid_type(int type, bool key) {
        if (type != LUA_TNUMBER && type != LUA_TSTRING) {
            if (key)
                return false;
            else
                return type == LUA_TBOOLEAN || type == LUA_TTABLE;
        }
        return true;
    }
    const int end_table_marker = LUA_TNONE;
    void write_object(std::ostream& stream, luabind::object obj) {
        using namespace luabind;
        for (iterator i(obj), end; i != end; ++i) {
            object key = i.key();
            object val = *i;
            if (!valid_type(type(key), true))
                continue;
            if (!valid_type(type(val), false))
                continue;
            // Write key
            write(stream, type(key));
            switch(type(key)) {
            case LUA_TNUMBER:
                write(stream, object_cast<double>(key));
                break;
            case LUA_TSTRING:
                write(stream, object_cast<std::string>(key));
                break;
            }
            // Write value
            write(stream, type(val));
            switch(type(val)) {
            case LUA_TBOOLEAN:
                write(stream, object_cast<bool>(val));
                break;
            case LUA_TNUMBER:
                write(stream, object_cast<double>(val));
                break;
            case LUA_TSTRING:
                write(stream, object_cast<std::string>(val));
                break;
            case LUA_TTABLE:
                write_object(stream, val);
                break;
            }
        }
        write(stream, end_table_marker);
    }
    luabind::object read_object(std::istream& stream, lua_State* state);
    luabind::object read_value(std::istream& stream, lua_State* state, int type_tag) {
        double temp_num;
        std::string temp_str;
        bool temp_bool;
        luabind::object value;
        switch(type_tag) {
        case LUA_TBOOLEAN:
            read(stream, temp_bool);
            value = luabind::object(state, temp_bool);
            break;
        case LUA_TNUMBER:
            read(stream, temp_num);
            value = luabind::object(state, temp_num);
            break;
        case LUA_TSTRING:
            read(stream, temp_str);
            value = luabind::object(state, temp_str);
            break;
        case LUA_TTABLE:
            value = read_object(stream, state);
            break;
        }
        return value;
    }
    luabind::object read_object(std::istream& stream, lua_State* state) {
        using namespace luabind;
        object obj = newtable(state);
        int type_tag;
        while (stream) {
            // Read key
            read(stream, type_tag);
            if (type_tag == end_table_marker)
                return obj;
            object key = read_value(stream, state, type_tag);
            // Read value
            read(stream, type_tag);
            object val = read_value(stream, state, type_tag);
            // Set key and value
            obj[key] = val;
        }
        return obj;
    }
}

Save_File::Save_File(lua_State* state, luabind::object data) :
    state(state), data(data) {}

std::ostream& operator<<(std::ostream& stream, const Save_File& save_file) {
    detail::write_object(stream, save_file.data);
    return stream;
}
std::istream& operator>>(std::istream& stream, Save_File& save_file) {
    save_file.data = detail::read_object(stream, save_file.state);
    return stream;
}
