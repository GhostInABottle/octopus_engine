#include "../../include/environments/default_environment.hpp"
#include "../../include/log.hpp"
#include "../../include/utility/file.hpp"

bool Default_Environment::open_store_page(Open_Page_Mode mode) {
    if (!can_open_store_page(mode)) {
        LOGGER_W << "Tried to open a store page in the default environment";
        return false;
    }

    return file_utilities::open_url(get_store_page_url());
}

bool Default_Environment::open_url(const std::string& url, Open_Page_Mode, const std::string& fallback_url) {
    auto success = file_utilities::open_url(url);
    if (success || fallback_url.empty()) return success;

    return file_utilities::open_url(fallback_url);
}
