#include "../../include/tests/game_fixture.hpp"
#include "../../include/utility/file.hpp"

std::unique_ptr<Game> Game_Fixture::game;

Game_Fixture::Game_Fixture() {
    file_utilities::user_data_folder();
    if (!game) {
        game = std::make_unique<Game>(std::vector<std::string>{}, nullptr);
    }
}
