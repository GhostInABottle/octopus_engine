#include "../include/rapidxml.hpp"
#include "../include/utility.hpp"
#include "../include/log.hpp"
#include "../include/game.hpp"
#include "../include/configurations.hpp"
#ifdef __APPLE__
#include <unistd.h>
#include "CoreFoundation/CoreFoundation.h"
#endif
#ifdef _WIN32
// Force high performance graphics on NVIDIA GPUs
#include <windows.h>
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

int main() {
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
        Configurations::parse("config.ini");
        LOGGER_I << "Reticulating Splines";
        Game::game_width = Configurations::get<int>("debug.width");
        Game::game_height = Configurations::get<int>("debug.height");
        Game game;
        game.run();

    } catch (const rapidxml::parse_error& e) {
        std::string where = e.where<char>();
        LOGGER_E << "RapidXml Exception: \"" << e.what() << "\" at \"" << where.substr(0, 50) << '"';
    } catch (const std::exception& e) {
        LOGGER_E << "Exception: " << e.what();
    } catch (...) {
        LOGGER_E << "Unknown exception!";
    }
}
