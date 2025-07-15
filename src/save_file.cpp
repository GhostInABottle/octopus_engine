#include "configurations.hpp"
#include "save_file.hpp"
#include "xd/vendor/sol/sol.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace detail {

    enum class type_tag : int {
        boolean = 1,
        double_number = 3,
        string = 4,
        table = 5,
        byte_number = 20,
        short_number = 21,
        long_number = 23,
        long_long_number = 24,
        float_number = 30,
        small_string = 31,
        unknown = 127,
        end_of_table = -1,
    };

    static type_tag get_numeric_type_tag(double value, bool compact) {
        if (!compact) return type_tag::double_number;

        auto can_be_int = !std::isnan(value) && !std::isinf(value)
            && std::trunc(value) == value;

        if (can_be_int) {
            if (value <= std::numeric_limits<std::int8_t>().max())
                return type_tag::byte_number;
            else if (value <= std::numeric_limits<std::int16_t>().max())
                return type_tag::short_number;
            else if (value <= std::numeric_limits<std::int32_t>().max())
                return type_tag::long_number;
            else if (value <= std::numeric_limits<std::int64_t>().max())
                return type_tag::long_long_number;
        }

        return value <= std::numeric_limits<float>().max()
            ? type_tag::float_number
            : type_tag::double_number;
    }

    static type_tag get_string_type_tag(const std::string& value, bool compact) {
        if (!compact) return type_tag::string;

        constexpr auto small_size = std::numeric_limits<unsigned char>().max();
        return value.length() <= small_size
            ? type_tag::small_string
            : type_tag::string;
    }

    static type_tag sol_type_to_type_tag(sol::type sol_type, const sol::object& value, bool compact) {
        switch (sol_type) {
        case sol::type::boolean:
            return type_tag::boolean;
        case sol::type::number:
            return get_numeric_type_tag(value.as<double>(), compact);
        case sol::type::string:
            return get_string_type_tag(value.as<std::string>(), compact);
        case sol::type::table:
            return type_tag::table;
        default:
            return type_tag::unknown;
        }
    }

    template<typename T>
    static void write(std::ostream& stream, const T& value) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    static void write(std::ostream& stream, const std::string& value, type_tag tag) {
        auto length = value.length();

        if (tag == type_tag::small_string)
            write(stream, static_cast<unsigned char>(length));
        else
            write(stream, length);

        stream.write(value.c_str(), length);
    }

    static void write_type_tag(std::ostream& stream, type_tag tag, bool compact) {
        if (compact)
            write(stream, static_cast<signed char>(tag));
        else
            write(stream, tag);
    }

    template <typename T>
    static T object_to_number(const sol::object& value) {
        auto double_value = value.as<double>();
        return static_cast<T>(double_value);
    }

    static void write_value(std::ostream& stream, type_tag tag, const sol::object& value) {
        switch (tag) {
        case type_tag::boolean:
            write(stream, value.as<bool>());
            break;
        case type_tag::string:
        case type_tag::small_string:
            write(stream, value.as<std::string>(), tag);
            break;
        case type_tag::byte_number:
            write(stream, object_to_number<std::int8_t>(value));
            break;
        case type_tag::short_number:
            write(stream, object_to_number<std::int16_t>(value));
            break;
        case type_tag::long_number:
            write(stream, object_to_number<std::int32_t>(value));
            break;
        case type_tag::long_long_number:
            write(stream, object_to_number<std::int64_t>(value));
            break;
        case type_tag::float_number:
            write(stream, object_to_number<float>(value));
            break;
        case type_tag::double_number:
            write(stream, value.as<double>());
            break;
        }
    }

    template<typename T>
    static void read(std::istream& stream, T& value){
        stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    static void read(std::istream& stream, std::string& value, type_tag tag) {
        std::string::size_type length;

        if (tag == type_tag::small_string) {
            unsigned char uc_length;
            read(stream, uc_length);
            length = static_cast<std::string::size_type>(uc_length);
        } else {
            read(stream, length);
        }

        value = std::string(length, ' ');
        stream.read(&value[0], length);
    }

    static void read_type_tag(std::istream& stream, type_tag& tag, bool compact) {
        if (compact) {
            signed char type;
            read(stream, type);
            tag = static_cast<type_tag>(type);
        }
        else {
            read(stream, tag);
        }
    }

    template <typename T>
    static sol::object read_numeric_value(std::istream& stream, const sol::state& state) {
        T temp{};
        read(stream, temp);
        return sol::make_object(state, static_cast<double>(temp));
    }

    static sol::object read_value(std::istream& stream, const sol::state& state, type_tag tag) {
        std::string temp_str;
        bool temp_bool;
        switch (tag) {
        case type_tag::boolean:
            read(stream, temp_bool);
            return sol::make_object(state, temp_bool);
        case type_tag::byte_number:
            return read_numeric_value<std::int8_t>(stream, state);
        case type_tag::short_number:
            return read_numeric_value<std::int16_t>(stream, state);
        case type_tag::long_number:
            return read_numeric_value<std::int32_t>(stream, state);
        case type_tag::long_long_number:
            return read_numeric_value<std::int64_t>(stream, state);
        case type_tag::float_number:
            return read_numeric_value<float>(stream, state);
        case type_tag::double_number:
            return read_numeric_value<double>(stream, state);
        case type_tag::string:
        case type_tag::small_string:
            read(stream, temp_str, tag);
            return sol::make_object(state, temp_str);
        default:
            throw std::runtime_error("Unexpected value type "
                + std::to_string(static_cast<int>(tag)));
        }
    }

    static bool valid_key_type(sol::type type) {
        return type == sol::type::number || type == sol::type::string;
    }

    static bool valid_value_type(sol::type type) {
        return type == sol::type::number
            || type == sol::type::string
            || type == sol::type::boolean
            || type == sol::type::table;
    }

    static void write_object(std::ostream& stream, sol::table obj, bool compact) {
        for (auto& kv : obj) {
            const auto& key = kv.first;
            const auto& val = kv.second;

            auto sol_key_type = key.get_type();
            auto sol_val_type = val.get_type();
            if (!valid_key_type(sol_key_type))
                continue;
            if (!valid_value_type(sol_val_type))
                continue;

            // Write the key
            auto key_type = sol_type_to_type_tag(sol_key_type, key, compact);
            write_type_tag(stream, key_type, compact);
            write_value(stream, key_type, key);

            // Write the value
            auto val_type = sol_type_to_type_tag(sol_val_type, val, compact);
            write_type_tag(stream, val_type, compact);

            switch(sol_val_type) {
            case sol::type::boolean:
            case sol::type::number:
            case sol::type::string:
                write_value(stream, val_type, val);
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
            write(stream, static_cast<signed char>(type_tag::end_of_table));
        else
            write(stream, type_tag::end_of_table);
    }

    static sol::table read_object(std::istream& stream, const sol::state& state, bool compact);

    static sol::table read_object(std::istream& stream, const sol::state& state, bool compact) {
        sol::table obj(state, sol::create);
        type_tag tag;
        while (stream) {
            // Read the key
            read_type_tag(stream, tag, compact);

            if (tag == type_tag::end_of_table) return obj;

            auto key = read_value(stream, state, tag);
            if (!key.valid()) throw std::runtime_error("Invalid object key");

            // Read the value
            read_type_tag(stream, tag, compact);

            auto val = tag == type_tag::table
                ? read_object(stream, state, compact)
                : read_value(stream, state, tag);

            // Set key and value
            obj[key] = val;
        }

        return obj;
    }
}

struct Save_File::Impl {
    Impl(sol::state& state, const sol::table* data, const sol::table* header, bool header_only, bool compact, bool valid) :
        state(state),
        data(data ? *data : sol::table{ state, sol::create }),
        header(header ? *header : sol::table{ state, sol::create }),
        header_only(header_only),
        compact(compact),
        valid(valid) {}
    sol::state& state;
    sol::table header;
    sol::table data;
    bool header_only;
    bool compact;
    bool valid;
};

// Reading constructor
Save_File::Save_File(sol::state& state, bool header_only, bool compact) :
    pimpl(std::make_unique<Impl>(state, nullptr, nullptr, header_only, compact, false)) {}

// Writing constructor
Save_File::Save_File(sol::state& state, const sol::table& data, const sol::table* header, bool compact) :
    pimpl(std::make_unique<Impl>(state, &data, header, false, compact, true)) {}

Save_File::~Save_File() {}

sol::object& Save_File::lua_data() { return pimpl->data; }

sol::object& Save_File::header_data() { return pimpl->header; }

bool Save_File::is_valid() const { return pimpl->valid; }

std::ostream& operator<<(std::ostream& stream, Save_File& save_file) {
    save_file.pimpl->valid = false;
    detail::write(stream, Configurations::get<unsigned int>("debug.save-signature"));
    auto compact = save_file.pimpl->compact;
    detail::write_object(stream, save_file.header_data(), compact);
    detail::write_object(stream, save_file.lua_data(), compact);
    save_file.pimpl->valid = true;
    return stream;
}

std::istream& operator>>(std::istream& stream, Save_File& save_file) {
    save_file.pimpl->valid = false;
    unsigned int signature;
    detail::read(stream, signature);
    if (signature != Configurations::get<unsigned int>("debug.save-signature")) {
        throw std::runtime_error("Invalid file signature");
    }

    auto& state = save_file.pimpl->state;
    auto compact = save_file.pimpl->compact;
    auto first_table = detail::read_object(stream, state, compact);

    if (save_file.pimpl->header_only) {
        save_file.pimpl->header = first_table;
        save_file.pimpl->valid = true;
        return stream;
    }

    if (stream.peek() != EOF) {
        // Headers are optional since they were introduced later
        save_file.pimpl->header = first_table;
        save_file.pimpl->data = detail::read_object(stream, state, compact);
    } else {
        save_file.pimpl->data = first_table;
    }

    save_file.pimpl->valid = true;
    return stream;
}
