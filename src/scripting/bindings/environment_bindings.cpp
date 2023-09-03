#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/environments/environment.hpp"

void bind_environment_types(sol::state& lua) {
    auto env_type = lua.new_usertype<Environment>("Environment");
    env_type["is_ready"] = sol::property(&Environment::is_ready);
    env_type["name"] = sol::property(&Environment::get_name);
    env_type["can_open_store_page"] = sol::property(&Environment::can_open_store_page);
    env_type["can_open_url"] = sol::property(&Environment::can_open_url);
    env_type["open_store_page"] = &Environment::open_store_page;
    env_type["open_url"] = &Environment::open_url;
}
