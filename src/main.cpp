#include "../include/vendor/rapidxml.hpp"
#include "../include/log.hpp"
#include "../include/game.hpp"
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
        file_utilities::game_data_filesystem(argv[1]);
        file_utilities::user_data_filesystem(argv[1]);

        // Parse the config file
        auto user_data_folder = file_utilities::user_data_folder();
        for (auto& warning : user_data_folder->get_warnings()) {
            LOGGER_I << warning;
        }
        user_data_folder->parse_config();
 
        // Initialize the audio system
        auto audio = std::make_shared<xd::audio>();

        LOGGER_I << "Reticulating Splines";

        Game game(args, audio);
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
