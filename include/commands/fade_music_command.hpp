#ifndef HPP_FADE_MUSIC_COMMAND
#define HPP_FADE_MUSIC_COMMAND

#include "../command.hpp"

namespace xd {
    class music;
}
class Game;

class Fade_Music_Command : public Command {
public:
    Fade_Music_Command(Game& game, xd::music& music, float volume, long duration);
    void execute();
    bool is_complete() const;
private:
    xd::music& music;
    Game& game;
    float old_volume;
    float new_volume;
    long start_time;
    long duration;
    bool complete;
};


#endif
