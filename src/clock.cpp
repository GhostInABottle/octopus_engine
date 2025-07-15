#include "clock.hpp"
#include "game.hpp"

Clock::Clock(Game& game) : game(game), start_time(game.ticks()), time_stop(false),
    stop_start_time(0), total_stopped_time(0) {}

int Clock::ticks() const {
    return game.ticks();
}

void Clock::stop_time() {
    if (time_stop)
        return;
    stop_start_time = game.ticks();
    time_stop = true;
}

void Clock::resume_time() {
    if (!time_stop)
        return;
    time_stop = false;
    total_stopped_time += game.ticks() - stop_start_time;
}

float Clock::seconds() const {
    int stopped_time = total_stopped_time +
        (time_stop ? game.ticks() - stop_start_time : 0);
    return (game.ticks() - stopped_time - start_time) / 1000.0f;
}
