#ifndef HPP_GAME_BINDINGS
#define HPP_GAME_BINDINGS

#include "../../xd/vendor/sol/forward.hpp"

class Game;

void bind_game_types(sol::state& lua, Game& game);

#endif
