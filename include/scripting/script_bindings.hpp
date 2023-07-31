#ifndef HPP_SCRIPT_BINDINGS
#define HPP_SCRIPT_BINDINGS

#include "../xd/vendor/sol/forward.hpp"

class Game;

void bind_utility_types(sol::state& lua, Game& game);
void bind_file_types(sol::state& lua, Game& game);
void bind_text_types(sol::state& lua, Game& game);
void bind_game_types(sol::state& lua, Game& game);
void bind_canvas_types(sol::state& lua, Game& game);
void bind_math_types(sol::state& lua);
void bind_map_object_types(sol::state& lua, Game& game);
void bind_audio_types(sol::state& lua, Game& game);
void bind_layer_types(sol::state& lua, Game& game);
void bind_map_types(sol::state& lua);
void bind_camera_types(sol::state& lua, Game& game);

#endif
