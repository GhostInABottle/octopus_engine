#include "../../include/commands/fade_music_command.hpp"
#include "../../include/game.hpp"
#include "../../include/utility.hpp"
#include <xd/audio.hpp>

Fade_Music_Command::Fade_Music_Command(Game& game, xd::music& music, float volume, long duration) :
    game(game),
    music(music),
    old_volume(music.get_volume()),
    new_volume(volume),
    start_time(game.ticks()),
    duration(duration) {}

void Fade_Music_Command::execute() {
    float alpha = calculate_alpha(game.ticks(), start_time, duration);
    music.set_volume(lerp(old_volume, new_volume, alpha));
}

bool Fade_Music_Command::is_complete() const {
    bool complete = stopped || game.ticks() - start_time > duration;
    if (complete)
        music.set_volume(new_volume);
    return complete;
}