#ifdef OCB_USE_STEAM_SDK
#ifndef HPP_STEAM_ENVIRONMENT
#define HPP_STEAM_ENVIRONMENT

#include "environment.hpp"

class Steam_Environment : public Environment {
public:
    Steam_Environment();
    ~Steam_Environment();
    virtual Open_Page_Mode get_preferred_open_page_mode() const override {
        return is_steam_deck
            ? Open_Page_Mode::OVERLAY
            : Open_Page_Mode::INTERNAL;
    }
    virtual bool is_ready() const override {
        return ready;
    }
    virtual bool should_restart() const { return restart; }
    virtual std::string get_name() const override;
    virtual bool can_open_store_page(Open_Page_Mode mode = Open_Page_Mode::ANY) const override {
        if (!ready) return false;

        if (mode == Open_Page_Mode::ANY) mode = get_preferred_open_page_mode();
        if (mode == Open_Page_Mode::INTERNAL) return app_id > 0;

        return !get_store_page_url().empty();
    }
    virtual bool open_store_page(Open_Page_Mode mode = Open_Page_Mode::ANY) override;
    virtual bool can_open_url(Open_Page_Mode mode = Open_Page_Mode::ANY) const override {
        return ready && mode != Open_Page_Mode::INTERNAL;
    }
    virtual bool open_url(const std::string& url, Open_Page_Mode mode = Open_Page_Mode::ANY,
        const std::string& fallback_url = "");
    virtual const Configurations::value_map get_preferred_configs() const override;
private:
    bool ready;
    bool restart;
    int app_id;
    bool is_steam_deck;
};

#endif
#endif