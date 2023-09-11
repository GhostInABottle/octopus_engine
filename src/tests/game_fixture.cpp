#include "../../include/tests/game_fixture.hpp"
#include "../../include/utility/file.hpp"

std::unique_ptr<Environment> Game_Fixture::environment;
std::unique_ptr<Game> Game_Fixture::game;

Game_Fixture::Game_Fixture() {
    if (!environment) {
        environment = std::make_unique<Default_Environment>();
    }
    file_utilities::user_data_folder(*environment);
    if (!game) {
        game = std::make_unique<Game>(std::vector<std::string>{}, nullptr, *environment);
    }
}
