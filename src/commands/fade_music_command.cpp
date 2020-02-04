#include "../../include/commands/fade_music_command.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/math.hpp"
#include "../../include/xd/audio.hpp"

Fade_Music_Command::Fade_Music_Command(Game& game, float volume, long duration) :
    game(game),
    music(game.playing_music()),
    old_volume(game.playing_music()->get_volume()),
    new_volume(volume),
    start_time(game.ticks()),
    duration(duration),
    complete(false) {}

void Fade_Music_Command::execute() {
    auto music_ptr = music.lock();
    complete = stopped || !music_ptr || game.ticks() - start_time > duration;
    if (!music_ptr) return;

    float alpha = complete ? 1.0f : calculate_alpha(game.ticks(), start_time, duration);
    music_ptr->set_volume(lerp(old_volume, new_volume, alpha));
}

bool Fade_Music_Command::is_complete() const {
    return complete;
}