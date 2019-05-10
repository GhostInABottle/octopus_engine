#include "../../include/tests/game_fixture.hpp"
#include "../../include/configurations.hpp"

std::unique_ptr<Game> Game_Fixture::game;

Game_Fixture::Game_Fixture() {
    Configurations::parse("config.ini");
    if (!game) {
        game.reset(new Game(nullptr));
    }
}
