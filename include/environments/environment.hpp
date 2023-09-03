#ifndef HPP_ENVIRONMENT
#define HPP_ENVIRONMENT

#include <string>

// A running environment and its SDK capabilities, e.g. Steam or console
class Environment {
public:
    // Is the environment initialized and ready to be used?
    // If this is false after construction, we'll fallback to the default env
    virtual bool is_ready() const = 0;
    // If set to true after the constructor, the game will exit so that
    // it could be restarted through the proper environment
    virtual bool should_restart() const { return false; }
    // The name of the environment for debugging purposes
    virtual std::string get_name() const = 0;
    // Try to open the store page on this environment
    virtual bool can_open_store_page() const = 0;
    virtual bool open_store_page() = 0;
    // Try to open a URL
    virtual bool can_open_url() const = 0;
    virtual bool open_url(const std::string& url) = 0;
    // Called when exiting the game
    virtual ~Environment() {}
};

#endif