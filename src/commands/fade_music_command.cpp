#include "../../include/audio_player.hpp"
#include "../../include/commands/fade_music_command.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/math.hpp"
#include "../../include/xd/audio.hpp"

Fade_Music_Command::Fade_Music_Command(Game& game, std::shared_ptr<xd::music> playing_music, float volume, long duration) :
        Timed_Command(game, duration),
        new_volume(volume),
        complete(false) {
    music = playing_music;
    old_volume = playing_music->get_volume();
}

void Fade_Music_Command::execute() {
    auto music_ptr = music.lock();
    complete = stopped || !music_ptr || is_done();
    if (!music_ptr) return;

    float alpha = get_alpha(complete);
    music_ptr->set_volume(lerp(old_volume, new_volume, alpha));
}

bool Fade_Music_Command::is_complete() const {
    return complete || force_stopped;
}
