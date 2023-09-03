#ifndef HPP_DEFAULT_ENVIRONMENT
#define HPP_DEFAULT_ENVIRONMENT

#include "environment.hpp"

class Default_Environment : public Environment {
public:
    virtual bool is_ready() const override {
        return true;
    }
    virtual std::string get_name() const override {
        return "DEFAULT";
    }
    virtual bool can_open_store_page() const override {
        return false;
    }
    virtual bool open_store_page() override;
    virtual bool can_open_url() const override {
        return true;
    }
    virtual bool open_url(const std::string& url);
};

#endif