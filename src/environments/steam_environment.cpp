#ifdef OCB_USE_STEAM_SDK
#include "../../include/environments/steam_environment.hpp"
#include "../../include/log.hpp"
#include "../../include/configurations.hpp"
#include "../../include/utility/file.hpp"
#include <steam/steam_api.h>

// TODO: Replace k_uAppIdInvalid with Steam AppID and remove steam_appid.txt

Steam_Environment::Steam_Environment() : ready(false), restart(false) {
    LOGGER_I << "Initializing Steam API";
    if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid)) {
        // Starts Steam and launches the game again.
        restart = true;
        return;
    }

    if (!SteamAPI_Init()) {
        LOGGER_I << "Steam failed to initialize.";
        return;
    }

    ready = true;
}

Steam_Environment::~Steam_Environment() {
    if (!ready) return;

    SteamAPI_Shutdown();
}

bool Steam_Environment::open_store_page() {
    if (!ready) {
        LOGGER_W << "Tried to open a store page, but Steam is not ready";
        return false;
    }

    auto app_id = Configurations::get<int>("steam.app-id");
    LOGGER_I << "Opening store page using Steam Overlay " << app_id;
    SteamFriends()->ActivateGameOverlayToStore(app_id, k_EOverlayToStoreFlag_None);
    return true;
}

bool Steam_Environment::open_url(const std::string& url) {
    if (!ready) {
        LOGGER_W << "Tried to open the url {" << url << "}, but Steam is not ready";
        return false;
    }

    LOGGER_I << "Opening URL using Steam Overlay " << url;
    SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
    return true;
}
#endif