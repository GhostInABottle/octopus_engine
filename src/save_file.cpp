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

    void write(std::ostream& stream, const std::string& value, bool compact){
        auto length = value.length();
        auto small = false;

        if (compact) {
            small = length < 255;
            write(stream, static_cast<unsigned char>(small ? 1 : 0));
        }

        if (small)
            write(stream, static_cast<unsigned char>(length));
        else
            write(stream, length);

        stream.write(value.c_str(), length);
    }

    void write_type_tag(std::ostream& stream, sol::type type_tag, bool compact) {
        if (compact)
            write(stream, static_cast<signed char>(type_tag));
        else
            write(stream, type_tag);
    }

    template<typename T>
    void read(std::istream& stream, T& value){
        stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    void read(std::istream& stream, std::string& value, bool compact) {
        std::string::size_type length;
        unsigned char small = 0;
        if (compact) {
            read(stream, small);
        }

        if (small == 1) {
            unsigned char uc_length;
            read(stream, uc_length);
            length = static_cast<std::string::size_type>(uc_length);
        } else {
            read(stream, length);
        }

        value = std::string(length, ' ');
        stream.read(&value[0], length);
    }

    void read_type_tag(std::istream& stream, sol::type& type_tag, bool compact) {
        if (compact) {
            signed char type;
            read(stream, type);
            type_tag = static_cast<sol::type>(type);
        }
        else {
            read(stream, type_tag);
        }
    }

    bool valid_key_type(sol::type type) {
        return type == sol::type::number || type == sol::type::string;
    }

    bool valid_value_type(sol::type type) {
        return type == sol::type::number
            || type == sol::type::string
            || type == sol::type::boolean
            || type == sol::type::table;
    }

    const sol::type end_table_marker = sol::type::none;

    void write_object(std::ostream& stream, sol::table obj, bool compact) {
        for (auto& kv : obj) {
            const auto& key = kv.first;
            const auto& val = kv.second;

            auto key_type = key.get_type();
            auto val_type = val.get_type();
            if (!valid_key_type(key_type))
                continue;
            if (!valid_value_type(val_type))
                continue;

            // Write the key
            write_type_tag(stream, key_type, compact);

            switch(key_type) {
            case sol::type::number:
                write(stream, key.as<double>());
                break;
            case sol::type::string:
                write(stream, key.as<std::string>(), compact);
                break;
            default:
                continue;
            }

            // Write the value
            write_type_tag(stream, val_type, compact);

            switch(val_type) {
            case sol::type::boolean:
                write(stream, val.as<bool>());
                break;
            case sol::type::number:
                write(stream, val.as<double>());
                break;
            case sol::type::string:
                write(stream, val.as<std::string>(), compact);
                break;
            case sol::type::table:
                write_object(stream, val, compact);
                break;
            default:
                throw std::runtime_error("Invalid object key");
            }
        }

        // Write end of table marker
        if (compact)
            write(stream, static_cast<signed char>(end_table_marker));
        else
            write(stream, end_table_marker);
    }

    sol::table read_object(std::istream& stream, const sol::state& state, bool compact);

    sol::object read_value(std::istream& stream, const sol::state& state, sol::type type_tag, bool compact) {
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
            read(stream, temp_str, compact);
            return sol::make_object(state, temp_str);
        case sol::type::table:
            return read_object(stream, state, compact);
        default:
            throw std::runtime_error("Unexpected value type");
        }
    }

    sol::table read_object(std::istream& stream, const sol::state& state, bool compact) {
        sol::table obj(state, sol::create);
        sol::type type_tag;
        while (stream) {
            // Read the key
            read_type_tag(stream, type_tag, compact);

            if (type_tag == end_table_marker)
                return obj;

            auto key = read_value(stream, state, type_tag, compact);
            if (!key.valid())
                throw std::runtime_error("Invalid object key");

            // Read the value
            read_type_tag(stream, type_tag, compact);
            auto val = read_value(stream, state, type_tag, compact);

            // Set key and value
            obj[key] = val;
        }

        return obj;
    }
}

Save_File::Save_File(sol::state& state, bool header_only, bool compact) :
    state(state),
    data(state, sol::create),
    header(state, sol::create),
    header_only(header_only),
    compact(compact),
    valid(false) {}

Save_File::Save_File(sol::state& state, const sol::table& data, std::optional<sol::table> header, bool compact) :
    state(state),
    data(data),
    header(header.value_or(sol::table(state, sol::create))),
    header_only(false),
    compact(compact),
    valid(true) {}

std::ostream& operator<<(std::ostream& stream, Save_File& save_file) {
    save_file.valid = false;
    detail::write(stream, Configurations::get<unsigned int>("debug.save-signature"));
    detail::write_object(stream, save_file.header, save_file.compact);
    detail::write_object(stream, save_file.data, save_file.compact);
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

    auto first_table = detail::read_object(stream, save_file.state, save_file.compact);

    if (save_file.header_only) {
        save_file.header = first_table;
        save_file.valid = true;
        return stream;
    }

    if (stream.peek() != EOF) {
        // Headers are optional since they were introduced later
        save_file.header = first_table;
        save_file.data = detail::read_object(stream, save_file.state, save_file.compact);
    } else {
        save_file.data = first_table;
    }

    save_file.valid = true;
    return stream;
}
