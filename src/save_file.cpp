#include "../include/save_file.hpp"
#include "../include/configurations.hpp"
#include <iostream>
#include <cstdio>
#include <string>
#include <stdexcept>

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
    bool valid_type(sol::type type, bool key) {
        if (type != sol::type::number && type != sol::type::string) {
            if (key)
                return false;
            else
                return type == sol::type::boolean || type == sol::type::table;
        }
        return true;
    }
    const sol::type end_table_marker = sol::type::none;
    void write_object(std::ostream& stream, sol::table obj) {
        for (auto& kv : obj) {
            const auto& key = kv.first;
            const auto& val = kv.second;
            if (!valid_type(key.get_type(), true))
                continue;
            if (!valid_type(val.get_type(), false))
                continue;

            // Write key
            write(stream, key.get_type());
            switch(key.get_type()) {
            case sol::type::number:
                write(stream, key.as<double>());
                break;
            case sol::type::string:
                write(stream, key.as<std::string>());
                break;
            default:
                continue;
            }

            // Write value
            write(stream, val.get_type());
            switch(val.get_type()) {
            case sol::type::boolean:
                write(stream, val.as<bool>());
                break;
            case sol::type::number:
                write(stream, val.as<double>());
                break;
            case sol::type::string:
                write(stream, val.as<std::string>());
                break;
            case sol::type::table:
                write_object(stream, val);
                break;
            default:
                throw std::runtime_error("Invalid object key");
            }
        }
        write(stream, end_table_marker);
    }
    sol::table read_object(std::istream& stream, const sol::state& state);
    sol::object read_value(std::istream& stream, const sol::state& state, sol::type type_tag) {
        double temp_num;
        std::string temp_str;
        bool temp_bool;
        switch(type_tag) {
        case sol::type::boolean:
            read(stream, temp_bool);
            return sol::make_object(state, temp_bool);
        case sol::type::number:
            read(stream, temp_num);
            return sol::make_object(state, temp_num);
        case sol::type::string:
            read(stream, temp_str);
            return sol::make_object(state, temp_str);
        case sol::type::table:
            return read_object(stream, state);
        default:
            throw std::runtime_error("Unexpected value type");
        }
    }
    sol::table read_object(std::istream& stream, const sol::state& state) {
        sol::table obj(state, sol::create);
        sol::type type_tag;
        while (stream) {
            // Read key
            read(stream, type_tag);
            if (type_tag == end_table_marker)
                return obj;

            auto key = read_value(stream, state, type_tag);
            if (!key.valid())
                throw std::runtime_error("Invalid object key");

            // Read value
            read(stream, type_tag);
            auto val = read_value(stream, state, type_tag);
            // Set key and value
            obj[key] = val;
        }
        return obj;
    }
}

Save_File::Save_File(sol::state& state, bool header_only) :
    state(state),
    data(state, sol::create),
    header(state, sol::create),
    valid(false),
    header_only(header_only) {}

Save_File::Save_File(sol::state& state, const sol::table& data, std::optional<sol::table> header) :
    state(state),
    data(data),
    header(header.value_or(sol::table(state, sol::create))),
    valid(true),
    header_only(false) {}

std::ostream& operator<<(std::ostream& stream, Save_File& save_file) {
    save_file.valid = false;
    detail::write(stream, Configurations::get<unsigned int>("debug.save-signature"));
    detail::write_object(stream, save_file.header);
    detail::write_object(stream, save_file.data);
    save_file.valid = true;
    return stream;
}

std::istream& operator>>(std::istream& stream, Save_File& save_file) {
    save_file.valid = false;
    unsigned int signature;
    detail::read(stream, signature);
    if (signature != Configurations::get<unsigned int>("debug.save-signature")) {
        throw std::runtime_error("Invalid file signature");
    }

    auto first_table = detail::read_object(stream, save_file.state);

    if (save_file.header_only) {
        save_file.header = first_table;
        save_file.valid = true;
        return stream;
    }

    if (stream.peek() != EOF) {
        // Headers are optional since they were introduced later
        save_file.header = first_table;
        save_file.data = detail::read_object(stream, save_file.state);
    } else {
        save_file.data = first_table;
    }

    save_file.valid = true;
    return stream;
}
