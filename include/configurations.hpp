#ifndef HPP_CONFIGURATIONS
#define HPP_CONFIGURATIONS

#include <variant>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>

class config_exception : public std::runtime_error {
public:
    config_exception(const std::string& error) : std::runtime_error(error) {}
};

// A class for dealing with configuration options read from file
class Configurations {
public:
    typedef std::function<void(const std::string&)> callback;
    // Load default values
    static void load_defaults();
    // Parse the configuration file and returns list of parse errors
    static std::vector<std::string> parse(std::string filename);
    // Get the option with given name
    template<typename T>
    static T get(const std::string& name) {
        if (values.find(name) != values.end())
            return std::get<T>(values[name]);
        else if (defaults.find(name) != defaults.end())
            return std::get<T>(defaults[name]);
        else
            throw config_exception(name + " is an invalid config value");
    }
    // Check if an option exists
    template<typename T>
    static bool contains_value(const std::string& name) {
        return values.find(name) != values.end();
    }
    // Get the option with given name as a string
    static std::string get_string(const std::string& name);
    // Add a callback to listen to config changes
    static void add_observer(const std::string& name, callback c) {
        observers[name] = c;
    }
    // Remove a callback with given name
    static void remove_observer(const std::string& name) {
        observers.erase(name);
    }
    // Update an option with given name
    template<typename T>
    static void set(const std::string& name, T value) {
        values[name] = value;
        for (auto& pair : observers) {
            pair.second(name);
        }
    }
private:
    typedef std::variant<std::string, int, unsigned int, float, bool> value_type;
    // Variable map to store the options
    static std::unordered_map<std::string, value_type> values;
    static std::unordered_map<std::string, value_type> defaults;
    static std::unordered_map<std::string, callback> observers;
};

#endif
