#include "../include/vendor/rapidxml.hpp"
#include "../include/log.hpp"
#include "../include/game.hpp"
#include "../include/utility/file.hpp"
#include "../include/xd/audio.hpp"

int main() {
    try {
        file_utilities::parse_config("config.ini");

        xd::audio audio;
        LOGGER_I << "Reticulating Splines";
        Game game(&audio);
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
