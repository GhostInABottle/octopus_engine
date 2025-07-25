#ifdef OCB_USE_STEAM_SDK
#include "steam_environment.hpp"
#include "../configurations.hpp"
#include "../log.hpp"
#include "../utility/file.hpp"
#include <steam/steam_api.h>

Steam_Environment::Steam_Environment()
        : ready(false)
        , restart(false)
        , app_id(Configurations::get<int>("steam.app-id")) {

    auto try_to_restart = Configurations::get<bool>("steam.restart-in-steam");
    if (try_to_restart && SteamAPI_RestartAppIfNecessary(app_id)) {
        // Starts Steam and launches the game again.
        restart = true;
        return;
    }

    if (!SteamAPI_Init()) {
        return;
    }

    ready = true;
}

Steam_Environment::~Steam_Environment() {
    if (!ready) return;

    SteamAPI_Shutdown();
}

Environment::Open_Page_Mode Steam_Environment::get_preferred_open_page_mode() const {
    if (!ready) return Open_Page_Mode::EXTERNAL;

    auto is_steam_deck = SteamUtils() && SteamUtils()->IsSteamRunningOnSteamDeck();
    return is_steam_deck ? Open_Page_Mode::OVERLAY : Open_Page_Mode::INTERNAL;
}

std::string Steam_Environment::get_user_id_string() const {
    if (!ready || !SteamUser()) return "";

    auto account_id = SteamUser()->GetSteamID().GetAccountID();
    return std::to_string(account_id);
}

std::string Steam_Environment::get_name() const {
    if (ready && SteamUtils()) {
        if (SteamUtils()->IsSteamRunningOnSteamDeck()) return "STEAM (DECK)";
        if (SteamUtils()->IsSteamInBigPictureMode()) return "STEAM (BIG PICTURE)";
    }

    return "STEAM";
}

bool Steam_Environment::open_store_page(Open_Page_Mode mode) {
    if (!can_open_store_page(mode)) {
        LOGGER_W << "Tried to open a store page in " << static_cast<int>(mode)
            << ", but Steam is not ready or mode is not supported";
        return false;
    }

    auto app_id = Configurations::get<int>("steam.app-id");

    if (mode == Open_Page_Mode::ANY) {
        mode = get_preferred_open_page_mode();
    }

    std::string url, fallback_url;
    if (mode == Open_Page_Mode::INTERNAL) {
        url = "steam://url/StoreAppPage/" + std::to_string(app_id);
        fallback_url = get_store_page_url();
        mode = Open_Page_Mode::EXTERNAL;
    } else {
        url = get_store_page_url();
    }

    // Could use ActivateGameOverlayToStore for overlay mode, but it doesn't have
    // positioning options and is more suited for adding DLC to the user's cart
    return open_url(url, mode, fallback_url);
}

bool Steam_Environment::open_url(const std::string& url, Open_Page_Mode mode,
        const std::string& fallback_url) {
    if (!can_open_url(mode)) {
        LOGGER_W << "Tried to open the url {" << url << "} in mode "
            << static_cast<int>(mode)
            << ", but Steam is not ready or mode is not supported";
        return false;
    }

    if (mode == Open_Page_Mode::OVERLAY && SteamFriends()) {
        LOGGER_I << "Opening URL using Steam Overlay " << url;
        SteamFriends()->ActivateGameOverlayToWebPage(url.c_str(),
            k_EActivateGameOverlayToWebPageMode_Modal);
        return true;
    }

    auto success = file_utilities::open_url(url);
    if (success || fallback_url.empty()) return success;

    return file_utilities::open_url(fallback_url);
}

const Configurations::value_map Steam_Environment::get_preferred_configs() const {
    if (!ready || !SteamUtils()) return {};

    Configurations::value_map configs;

    auto is_steam_deck = SteamUtils()->IsSteamRunningOnSteamDeck();
    auto is_big_picture = SteamUtils()->IsSteamInBigPictureMode();
    if (is_steam_deck || is_big_picture) {
        configs["graphics.fullscreen"] = true;
    }

    if (is_steam_deck) {
        configs["graphics.scale-mode"] = std::string{"stretch"};
    }

    return configs;
}

#endif
