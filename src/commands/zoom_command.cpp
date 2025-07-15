#include "zoom_command.hpp"
#include "../game.hpp"
#include "../utility/math.hpp"

Zoom_Command::Zoom_Command(Game& game, float magnification, long duration) :
    Timed_Command(game, duration), old_magnification(game.get_magnification()),
    new_magnification(magnification), complete(false) {}

void Zoom_Command::execute() {
    complete = stopped || is_done();
    float alpha = get_alpha(complete);
    float mag = lerp(old_magnification, new_magnification, alpha);
    if (!check_close(mag, game.get_magnification()))
        game.set_magnification(mag);
}

bool Zoom_Command::is_complete() const {
    return complete || force_stopped;
}
