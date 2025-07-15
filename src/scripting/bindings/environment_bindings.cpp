#include "../script_bindings.hpp"
#include "../../environments/environment.hpp"
#include "../../xd/vendor/sol/sol.hpp"

void bind_environment_types(sol::state& lua) {
    // Open page modes
    lua.new_enum<Environment::Open_Page_Mode>("Open_Page_Mode",
        {
            { "any", Environment::Open_Page_Mode::ANY },
            { "external", Environment::Open_Page_Mode::EXTERNAL },
            { "internal", Environment::Open_Page_Mode::INTERNAL },
            { "overlay", Environment::Open_Page_Mode::OVERLAY },
        }
    );

    auto env_type = lua.new_usertype<Environment>("Environment");
    env_type["name"] = sol::property(&Environment::get_name);

    env_type["can_open_store_page"] = [](Environment& env, std::optional<Environment::Open_Page_Mode> mode) {
        return env.can_open_store_page(mode.value_or(Environment::Open_Page_Mode::ANY));
    };
    env_type["open_store_page"] = [](Environment& env, std::optional<Environment::Open_Page_Mode> mode) {
        return env.open_store_page(mode.value_or(Environment::Open_Page_Mode::ANY));
    };

    env_type["can_open_url"] = [](Environment& env, std::optional<Environment::Open_Page_Mode> mode) {
        return env.can_open_url(mode.value_or(Environment::Open_Page_Mode::ANY));
    };
    env_type["open_url"] = [](Environment& env, const std::string& url, std::optional<Environment::Open_Page_Mode> mode) {
        return env.open_url(url, mode.value_or(Environment::Open_Page_Mode::ANY));
    };
}
