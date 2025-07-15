#ifndef HPP_ENVIRONMENT
#define HPP_ENVIRONMENT

#include "../configurations.hpp"
#include <string>

// A running environment and its SDK capabilities, e.g. Steam or console
class Environment {
public:
    // Indicates how to open store pages or URLs
    enum class Open_Page_Mode {
        ANY, // The environment's preferred mode
        EXTERNAL, // Launch the default browser
        INTERNAL, // Open through the environment's app/client
        OVERLAY, // Open as an overlay on top of the game
    };
    // Get the preferred mode for opening URLs
    virtual Open_Page_Mode get_preferred_open_page_mode() const = 0;
    // Get the current user's ID as a string (empty if it's not applicable)
    virtual std::string get_user_id_string() const { return ""; }
    // Is the environment initialized and ready to be used?
    // If this is false after construction, we'll fallback to the default env
    virtual bool is_ready() const = 0;
    // If set to true after the constructor, the game will exit so that
    // it could be restarted through the proper environment
    virtual bool should_restart() const { return false; }
    // The name of the environment for debugging purposes
    virtual std::string get_name() const = 0;
    // Get configured store URL
    std::string get_store_page_url() const {
        return Configurations::get<std::string>("game.store-url");
    }
    // Try to open the store page on this environment
    virtual bool can_open_store_page(Open_Page_Mode mode = Open_Page_Mode::ANY) const = 0;
    virtual bool open_store_page(Open_Page_Mode mode = Open_Page_Mode::ANY) = 0;
    // Try to open a URL
    virtual bool can_open_url(Open_Page_Mode mode = Open_Page_Mode::ANY) const = 0;
    virtual bool open_url(const std::string& url, Open_Page_Mode mode = Open_Page_Mode::ANY,
        const std::string& fallback_url = "") = 0;
    // Does the environment have some preferred configurations?
    virtual const Configurations::value_map get_preferred_configs() const {
        return {};
    }
    // Called when exiting the game
    virtual ~Environment() {}
};

#endif