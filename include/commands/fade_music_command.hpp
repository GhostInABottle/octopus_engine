#ifndef HPP_FADE_MUSIC_COMMAND
#define HPP_FADE_MUSIC_COMMAND

#include <memory>
#include "timed_command.hpp"

namespace xd {
    class music;
}
class Game;

class Fade_Music_Command : public Timed_Command {
public:
    Fade_Music_Command(Game& game, float volume, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    std::weak_ptr<xd::music> music;
    float old_volume;
    float new_volume;
    bool complete;
};

#endif
