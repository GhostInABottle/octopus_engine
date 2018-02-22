#include "../../include/commands/shake_screen_command.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"

Shake_Screen_Command::Shake_Screen_Command(Game& game, float strength, float speed, long duration) :
    game(game),
    start_time(game.ticks()),
    duration(duration) {
    game.get_camera()->start_shaking(strength, speed);
}

bool Shake_Screen_Command::is_complete() const {
    bool complete = stopped || game.ticks() - start_time > duration;
    if (complete)
        game.get_camera()->cease_shaking();
    return complete;
}