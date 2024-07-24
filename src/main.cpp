#include "../include/environments/default_environment.hpp"
#include "../include/game.hpp"
#include "../include/log.hpp"
#include "../include/vendor/rapidxml.hpp"
#ifdef OCB_USE_STEAM_SDK
#include "../include/environments/steam_environment.hpp"
#endif
#include "../include/utility/file.hpp"
#include "../include/xd/audio/audio.hpp"
#include <string>
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

        // Parse the config.ini file in the executable directory
        // (needed for archive path and Steam app ID)
        auto default_logs = User_Data_Folder::parse_default_config();

        // Initialize the environment
        Default_Environment default_environment;
        Environment* environment = &default_environment;
#ifdef OCB_USE_STEAM_SDK
        Steam_Environment steam_environment;
        environment = &steam_environment;
#endif

        auto should_restart = environment->should_restart();
        auto use_default = environment != &default_environment
            && !environment->is_ready();
        if (use_default) {
            // Fall back to default environment
            environment = &default_environment;
        }

        Log::set_environment(environment);

        // Initialize the filesystems (executable name is needed by PhysFS)
        file_utilities::default_config_filesystem(argv[0]);
        file_utilities::game_data_filesystem(argv[0]);
        file_utilities::user_data_filesystem(argv[0]);

        // Parse the user's config file
        auto user_data_folder = file_utilities::user_data_folder(*environment);
        user_data_folder->parse_config();

        // Safe to log now that the correct log file configs are loaded
        LOGGER_I << "Reticulating Splines";

        for (auto& [level, message] : default_logs) {
            Log(level) << message;
        }

        for (auto& [level, message] : user_data_folder->get_logs()) {
            Log(level) << message;
        }
        user_data_folder->clear_logs();

        // Check if the environment wanted to launch the executable
        if (should_restart) {
            LOGGER_I << "Environment requested a restart. Exiting...";
            return 0;
        }

        // Log earlier non-default environment failure
        if (use_default) {
            LOGGER_W << "Environment " << environment->get_name()
                << " failed to initialize. Falling back to default";
        } else {
            LOGGER_I << "Started in " << environment->get_name() << " environment";
        }

        // Initialize the audio system
        LOGGER_I << "Initializing the audio system";
        auto audio = std::make_shared<xd::audio>();

        auto preferred_configs = environment->get_preferred_configs();
        for (auto& [key, value] : preferred_configs) {
            Configurations::override_value(key, value);
        }

        LOGGER_I << "Creating the window";
        Game game(args, audio, *environment);

        LOGGER_I << "Initializing...";
        game.init();

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
