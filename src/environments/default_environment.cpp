#include "../../include/environments/default_environment.hpp"
#include "../../include/log.hpp"
#include "../../include/utility/file.hpp"

bool Default_Environment::open_store_page() {
    LOGGER_W << "Tried to open a store page in the default environment";
    return false;
}

bool Default_Environment::open_url(const std::string& url) {
    return file_utilities::open_url(url);
}
