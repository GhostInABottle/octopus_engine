#ifndef HPP_TIMED_COMMAND
#define HPP_TIMED_COMMAND

#include "../command.hpp"

class Game;

class Timed_Command : public Command {
public:
    Timed_Command(Game& game, int duration, int start_time = -1);
    void pause(int ticks = -1) noexcept override;
    void resume() noexcept override;
    long paused_time(int ticks = -1) const;
    long passed_time(int ticks = -1) const;
    bool is_done(int ticks = -1) const;
    float get_alpha(bool complete, int ticks = -1) const;
    virtual ~Timed_Command() = 0;
protected:
    Game& game;
    int start_time;
    int duration;
    int pause_start;
};

#endif
