#ifndef HPP_CONFIGURATIONS
#define HPP_CONFIGURATIONS

#include <variant>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>
#include <iosfwd>
#include "log.hpp"

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
    static std::vector<std::string> parse(std::istream& stream);
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
            return std::get<T>(defaults[name]);
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
    static void set(const std::string& name, T value) {
        if (exists(name) && get<T>(name) == value) {
            return;
        }

        values[name] = value;
        for (auto& pair : observers) {
            pair.second(name);
        }

        changed_since_save = true;

        // Log level is usually changed to suppress logs, so don't log it
        if (name == "logging.level") return;
        LOGGER_I << "Config " << name << " changed to " << value;
    }
    // Directly set a value without notifying observers or logging
    template<typename T>
    static void override_value(const std::string& name, T value) {
        values[name] = value;
        changed_since_save = true;
    }
private:
    inline static bool changed_since_save = false;
    // Variable map to store the options
    inline static value_map values;
    inline static value_map defaults;
    // Order of the parsed config file
    typedef std::tuple<std::string, std::vector<std::string>> section_ordered_values;
    inline static std::vector<section_ordered_values> section_order;
    // Observers to be called when the config changes
    inline static std::unordered_map<std::string, callback> observers;
};

#endif
