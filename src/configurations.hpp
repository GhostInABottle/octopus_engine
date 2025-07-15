#ifndef HPP_CONFIGURATIONS
#define HPP_CONFIGURATIONS

#include "log.hpp"
#include <functional>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

class config_exception : public std::runtime_error {
public:
    config_exception(const std::string& error) : std::runtime_error(error) {}
};

// A class for dealing with configuration options read from file
class Configurations {
public:
    typedef std::variant<std::string, int, unsigned int, float, bool> value_type;
    typedef std::unordered_map<std::string, value_type> value_map;
    typedef std::function<void(const std::string&)> callback;
    // Load default values
    static void load_defaults();
    // Check if configuration defaults are loaded
    static bool defaults_loaded() noexcept { return !defaults.empty(); }
    // Parse the configuration file and returns list of parse errors
    static std::vector<std::string> parse(std::istream& stream, bool is_default);
    // Save the configuration file
    static void save(std::ostream& stream);
    // Has the configurations changed since it was last saved?
    static bool changed() noexcept { return changed_since_save; }
    // Get the option with given name
    template<typename T>
    static T get(const std::string& name) {
        if (has_value(name)) {
            return std::get<T>(values[name]);
        } else if (has_default(name)) {
            return std::get<T>(defaults[name].value);
        }

        throw config_exception(name + " is an invalid config value");
    }
    // Try to get explicitly configured value for given name, or fallback to default
    template<typename T>
    static T get(const std::string& name, const std::string& fallback) {
        if (!has_value(name) && has_value(fallback)) {
            return get<T>(fallback);
        }

        return get<T>(name);
    }
    // Check if an option exists
    static bool has_value(const std::string& name) {
        return values.find(name) != values.end();
    }
    // Check if option has a default value
    static bool has_default(const std::string& name) {
        return defaults.find(name) != defaults.end();
    }
    // Check if config exists
    static bool exists(const std::string& name) {
        return has_value(name) || has_default(name);
    }
    // Get the option with given name as a string
    static std::string get_string(const std::string& name);
    // Add a callback to listen to config changes
    static void add_observer(const std::string& name, callback c) {
        observers[name] = c;
    }
    // Remove a callback with given name
    static void remove_observer(const std::string& name) noexcept {
        observers.erase(name);
    }
    // Update an option with given name
    template<typename T>
    static void set(const std::string& name, T value, bool log = true) {
        bool value_exists = exists(name);
        if (value_exists && get<T>(name) == value) {
            return;
        }

        if (has_default(name) && !defaults[name].modifiable) {
            LOGGER_W << "Tried to modify non-modifiable config: " << name;
            return;
        }

        values[name] = value;

        for (auto& pair : observers) {
            pair.second(name);
        }

        if (log) {
            LOGGER_I << "Config " << name << " changed to " << value;
        }

        // Add any new values to the end of the ordered section (for saving)
        if (!value_exists) {
            add_to_section_order(name);
        }

        changed_since_save = true;
    }
    // Directly set a value without notifying observers or logging
    template<typename T>
    static void override_value(const std::string& name, T value) {
        if (has_default(name) && !defaults[name].modifiable) {
            LOGGER_W << "Tried to modify non-modifiable config: " << name;
            return;
        }

        values[name] = value;

        if (!exists(name)) {
            add_to_section_order(name);
        }

        changed_since_save = true;
    }
private:
    struct Default {
        value_type value;
        // If false, value can't be set except when loading the default config file
        bool modifiable;
        Default(value_type value = false, bool modifiable = true)
            : value(value), modifiable(modifiable) {}
    };
    inline static bool changed_since_save = false;
    // Variable map to store the options
    inline static value_map values;
    // Default values and their properties
    inline static std::unordered_map<std::string, Default> defaults;
    // Order of the parsed config file
    typedef std::tuple<std::string, std::vector<std::string>> section_ordered_values;
    typedef std::vector<section_ordered_values> section_order_list;
    inline static section_order_list section_order;
    // Observers to be called when the config changes
    inline static std::unordered_map<std::string, callback> observers;
    // Add an ordered section/keys tuple
    static section_order_list::iterator add_section(const std::string& section, bool should_add) {
        if (!should_add) return section_order.end();

        section_order.push_back({ section, std::vector<std::string>{} });
        return section_order.end() - 1;
    }
    // Add an ordered key to the section/keys tuple
    static void add_to_section_order(section_ordered_values& section_values,
            const std::string& value, bool should_add) {
        if (!should_add) return;

        auto& keys = std::get<1>(section_values);
        keys.push_back(value);
    }
    // Find the ordered tuple for a named section
    static section_order_list::iterator find_ordered_section(const std::string& section_name) {
        for (auto i = std::begin(section_order); i < std::end(section_order); ++i) {
            if (std::get<0>(*i) == section_name) return i;
        }

        return std::end(section_order);
    }
    // Add a section if it's missing
    static void add_to_section_order(const std::string& config_key) {
        std::string section, key;
        auto first_dot = config_key.find_first_of('.');
        if (first_dot != std::string::npos) {
            section = config_key.substr(0, first_dot);
            key = config_key.substr(first_dot + 1);
        } else {
            key = section;
        }

        auto section_iterator = find_ordered_section(section);
        if (section_iterator == section_order.end()) {
            section_iterator = add_section(section, true);
            if (section_iterator == section_order.end()) return;
        }

        add_to_section_order(*section_iterator, config_key, true);
    }
};

#endif
