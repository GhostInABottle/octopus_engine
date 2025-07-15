#ifndef HPP_GAME_FIXTURE
#define HPP_GAME_FIXTURE

#include "../game.hpp"
#include "../environments/environment.hpp"
#include <memory>

struct Game_Fixture
{
    Game_Fixture();

    static std::unique_ptr<Environment> environment;
    static std::unique_ptr<Game> game;
};

#endif
