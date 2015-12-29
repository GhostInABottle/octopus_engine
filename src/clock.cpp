#include "../include/clock.hpp"
#include "../include/game.hpp"
#include "../include/configurations.hpp"

Clock::Clock(Game& game) : game(game), start_time(game.ticks()), time_stop(false),
    stop_start_time(0), total_stopped_time(0), added_time(0),
    time_multiplier(Configurations::get<float>("debug.time-multiplier")) {}

int Clock::ticks() const {
    return game.ticks();
}
void Clock::stop_time() {
    if (time_stop)
        return;
    stop_start_time = game.ticks();
    time_stop = true;
}
// Resume game time
void Clock::resume_time() {
    if (!time_stop)
        return;
    time_stop = false;
    total_stopped_time += game.ticks() - stop_start_time;
}
// Get total time in seconds without applying a multipler
int Clock::actual_seconds() const {
    int stopped_time = total_stopped_time +
        (time_stop ? game.ticks() - stop_start_time : 0);
    return (game.ticks() + added_time - stopped_time - start_time) / 1000;
}

void Clock::add_seconds(int seconds) {
    added_time += static_cast<int>(seconds * 1000.0f / time_multiplier);
}
