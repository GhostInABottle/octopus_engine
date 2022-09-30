#ifndef HPP_CANVAS_BINDINGS
#define HPP_CANVAS_BINDINGS

#include "../../xd/vendor/sol/forward.hpp"

class Game;

void bind_canvas_types(sol::state& lua, Game& game);

#endif
