#ifndef HPP_CONFIGURATIONS
#define HPP_CONFIGURATIONS

#include <boost/property_tree/ptree.hpp>
#include <boost/any.hpp>
#include <unordered_map>

// A class for dealing with configuration options read from file
class Configurations {
public:
    // Parse the configuration file
    static void parse(const std::string& filename);
    // Get the option with given name
    template<typename T>
    static T get(const std::string& name) {
        if (defaults.find(name) != defaults.end())
            return pt.get<T>(name, boost::any_cast<T>(defaults[name]));
        else
            return pt.get<T>(name);
    }
    // Get the option with given name as a string
    static std::string get_string(const std::string& name);
private:
    // Variable map to store the options
    static boost::property_tree::ptree pt;
    static std::unordered_map<std::string, boost::any> defaults;
};

#endif
