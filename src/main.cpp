#include "../include/vendor/rapidxml.hpp"
#include "../include/log.hpp"
#include "../include/game.hpp"
#include "../include/environments/default_environment.hpp"
#ifdef OCB_USE_STEAM_SDK
#include "../include/environments/steam_environment.hpp"
#endif
#include "../include/utility/file.hpp"
#include "../include/xd/audio.hpp"
#include <vector>
#include <memory>
#ifdef __APPLE__
#include <unistd.h>
#include "CoreFoundation/CoreFoundation.h"
#endif

int main(int argc, char* argv[]) {
    try {
#ifdef __APPLE__
        // Set current working directory to resources folder in OSX
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        char path[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8*)path, PATH_MAX)) {
            CFRelease(resourcesURL);
            chdir(path);
        } else {
            throw std::runtime_error("Unable to get OSX resources folder");
        }
#endif

        std::vector<std::string> args{ argv, argv + argc };

        // Init the filesystems
        file_utilities::default_config_filesystem(argv[0]);
        file_utilities::game_data_filesystem(argv[0]);
        file_utilities::user_data_filesystem(argv[0]);

        // Parse the config file
        auto user_data_folder = file_utilities::user_data_folder();
        for (auto& warning : user_data_folder->get_warnings()) {
            LOGGER_I << warning;
        }
        user_data_folder->parse_config();

        LOGGER_I << "Reticulating Splines";
 
        // Initialize the audio system
        LOGGER_I << "Initializing the audio system";
        auto audio = std::make_shared<xd::audio>();

        // Initialize the environment
        LOGGER_I << "Initializing the environment";
#ifdef OCB_USE_STEAM_SDK
        auto environment = std::make_shared<Steam_Environment>();
#else
        auto environment = std::make_shared<Default_Environment>();
#endif

        auto preferred_configs = environment->get_preferred_configs();
        for (auto& [key, value] : preferred_configs) {
            Configurations::override_value(key, value);
        }

        if (environment->should_restart()) {
            LOGGER_I << "Environment requested a restart. Exiting...";
            return 0;
        }

        LOGGER_I << "Initializing the god object";
        Game game(args, audio, environment);

        LOGGER_I << "We have a liftoff!";
        game.run();

        LOGGER_I << "Splines Reticulated";
    } catch (const rapidxml::parse_error& e) {
        std::string where = e.where<char>();
        LOGGER_E << "RapidXml Exception: \"" << e.what() << "\" at \"" << where.substr(0, 50) << '"';
    } catch (const std::exception& e) {
        LOGGER_E << "Exception: " << e.what();
    } catch (...) {
        LOGGER_E << "Unknown exception!";
    }
}
