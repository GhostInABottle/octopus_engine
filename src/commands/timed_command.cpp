#include "timed_command.hpp"
#include "../game.hpp"
#include "../utility/math.hpp"

Timed_Command::Timed_Command(Game& game, int duration, int start_time) :
    game(game),
    game_was_paused(game.is_paused()),
    start_time(start_time < 0 ? get_ticks() : start_time),
    duration(duration),
    pause_start(-1) {}

Timed_Command::~Timed_Command() {}

void Timed_Command::reset(int duration, int start_time) {
    game_was_paused = game.is_paused();
    this->start_time = start_time < 0 ? get_ticks() : start_time;
    this->duration = duration;
    pause_start = -1;
    stopped = false;
    paused = false;
}

void Timed_Command::pause(int ticks) noexcept {
    Command::pause();
    pause_start = ticks < 0 ? get_ticks() : ticks;
}

void Timed_Command::resume() noexcept {
    Command::resume();
    pause_start = -1;
}

long Timed_Command::paused_time(int ticks) const {
    if (pause_start == -1) return 0;
    if (ticks < 0) ticks = get_ticks();

    return ticks - pause_start;
}

long Timed_Command::passed_time(int ticks) const {
    if (ticks < 0) ticks = get_ticks();

    return ticks - start_time - paused_time(ticks);
}

bool Timed_Command::is_done(int ticks) const {
    return passed_time(ticks) > duration;
}

float Timed_Command::get_alpha(bool complete, int ticks) const {
    if (ticks < 0) ticks = get_ticks();

    return complete ? 1.0f : calculate_alpha(ticks - paused_time(), start_time, duration);
}

int Timed_Command::get_ticks() const {
    return game_was_paused ? game.window_ticks() : game.ticks();
}
