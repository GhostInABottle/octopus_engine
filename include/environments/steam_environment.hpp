#ifdef OCB_USE_STEAM_SDK
#ifndef HPP_STEAM_ENVIRONMENT
#define HPP_STEAM_ENVIRONMENT

#include "environment.hpp"

class Steam_Environment : public Environment {
public:
    Steam_Environment();
    ~Steam_Environment();
    virtual bool is_ready() const override {
        return ready;
    }
    virtual bool should_restart() const { return restart; }
    virtual std::string get_name() const override {
        return "STEAM";
    }
    virtual bool can_open_store_page() const override {
        return ready;
    }
    virtual bool open_store_page() override;
    virtual bool can_open_url() const override {
        return ready;
    }
    virtual bool open_url(const std::string& url);
private:
    bool ready;
    bool restart;
};

#endif
#endif