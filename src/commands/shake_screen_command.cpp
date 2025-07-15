#include "shake_screen_command.hpp"
#include "../camera.hpp"
#include "../game.hpp"

Shake_Screen_Command::Shake_Screen_Command(Game& game, xd::vec2 strength, xd::vec2 speed, long duration) : Timed_Command(game, duration) {
    game.get_camera()->start_shaking(strength, speed);
}

bool Shake_Screen_Command::is_complete() const {
    bool complete = stopped || is_done();
    if (complete) {
        game.get_camera()->cease_shaking();
    }

    return complete;
}
