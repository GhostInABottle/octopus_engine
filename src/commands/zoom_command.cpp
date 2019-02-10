#include "../../include/commands/zoom_command.hpp"
#include "../../include/game.hpp"
#include "../../include/utility.hpp"

Zoom_Command::Zoom_Command(Game& game, float magnification, long duration) :
    game(game), old_magnification(game.get_magnification()),
    new_magnification(magnification), start_time(game.ticks()),
    duration(duration), complete(false) {}

void Zoom_Command::execute() {
    complete = stopped || game.ticks() - start_time > duration;
    float alpha = complete ? 1.0f : calculate_alpha(game.ticks(), start_time, duration);
    float mag = lerp(old_magnification, new_magnification, alpha);
    if (!check_close(mag, game.get_magnification()))
        game.set_magnification(mag);
}

bool Zoom_Command::is_complete() const {
    return complete;
}