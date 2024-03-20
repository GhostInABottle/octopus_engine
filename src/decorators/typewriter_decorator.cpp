#include "../../include/audio_player.hpp"
#include "../../include/decorators/typewriter_decorator.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/xd/graphics/text_formatter.hpp"
#include <cstdlib>

Typewriter_Decorator::Typewriter_Decorator(Game& game, Audio_Player& audio_player)
    : game(game), audio_player(audio_player) {}

void Typewriter_Decorator::operator()(xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args) {
    // The slot to use (if multiple typewriter texts are shown, they should use different slots)
    auto slot = args.get<int>(0, 0);

    // The delay for showing each character in ms
    auto delay = args.get<int>(1, 100);
    if (delay == -1) delay = 100;

    // The sound effect for printing characters
    auto sound_filename = args.get<std::string>(2, "");
    auto has_sound = !sound_filename.empty();
    auto sound_volume = args.get<float>(3, 1.0f);
    auto sound_pitch = args.get<float>(4, 1.0f);
    auto sound_max_pitch = args.get<float>(5, 1.0f);
    if (sound_pitch < sound_max_pitch) {
        sound_pitch += (sound_max_pitch - sound_pitch)
            * (std::rand() / static_cast<float>(RAND_MAX));
    }

    auto new_state = states.find(slot) == states.end();
    auto& state = states[slot];

    auto unformatted = text.get_unformatted();
    if (new_state || state.text != unformatted) {
        state.game_paused = game.is_paused();
        state.start_time = state.ticks(game);
        state.text = unformatted;
        state.done = false;
        state.last_index = 0;
    }

    auto elapsed = state.ticks(game) - state.start_time;

    int index = 0;
    xd::formatted_text::const_iterator i;
    for (i = text.begin(); i != text.end(); ++i) {
        if (!state.done && elapsed / delay < ++index) break;

        decorator.push_text(*i);

        // If a sound effect is specified, play it for each new character
        if (!has_sound || index <= state.last_index) continue;
        state.last_index = index;
        audio_player.play_sound(game, sound_filename, sound_pitch, sound_volume);
    }

    if (!state.done && i == text.end()) {
        state.done = true;
    }
}

int Typewriter_Decorator::State::ticks(Game& game) const {
    return game_paused ? game.window_ticks() : game.ticks();
}
