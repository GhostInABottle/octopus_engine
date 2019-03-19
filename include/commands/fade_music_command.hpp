#ifndef HPP_FADE_MUSIC_COMMAND
#define HPP_FADE_MUSIC_COMMAND

#include <memory>
#include "../command.hpp"

namespace xd {
    class music;
}
class Game;

class Fade_Music_Command : public Command {
public:
    Fade_Music_Command(Game& game, float volume, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Game& game;
    std::weak_ptr<xd::music> music;
    float old_volume;
    float new_volume;
    long start_time;
    long duration;
    bool complete;
};


#endif
