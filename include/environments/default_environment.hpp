#ifndef HPP_DEFAULT_ENVIRONMENT
#define HPP_DEFAULT_ENVIRONMENT

#include "environment.hpp"

class Default_Environment : public Environment {
public:
    virtual Open_Page_Mode get_preferred_open_page_mode() const override {
        return Open_Page_Mode::EXTERNAL;
    }
    virtual bool is_ready() const override {
        return true;
    }
    virtual std::string get_name() const override {
        return "DEFAULT";
    }
    virtual bool can_open_store_page(Open_Page_Mode mode = Open_Page_Mode::ANY) const override {
        auto store_url = get_store_page_url();
        return !store_url.empty() && can_open_url(mode);
    }
    virtual bool open_store_page(Open_Page_Mode mode = Open_Page_Mode::ANY) override;
    virtual bool can_open_url(Open_Page_Mode mode = Open_Page_Mode::ANY) const override {
        return mode == Open_Page_Mode::EXTERNAL || mode == Open_Page_Mode::ANY;
    }
    virtual bool open_url(const std::string& url, Open_Page_Mode mode = Open_Page_Mode::ANY,
        const std::string& fallback_url = "");
};

#endif