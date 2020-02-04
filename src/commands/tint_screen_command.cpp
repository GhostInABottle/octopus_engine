#include "../../include/commands/tint_screen_command.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/utility/math.hpp"

Tint_Screen_Command::Tint_Screen_Command(Game& game, xd::vec4 color, long duration) :
    game(game), old_color(game.get_camera()->get_tint_color()), new_color(color),
    start_time(game.ticks()), duration(duration), complete(false) {}

void Tint_Screen_Command::execute() {
    complete = stopped || game.ticks() - start_time > duration;
    float alpha = complete ? 1.0f : calculate_alpha(game.ticks(), start_time, duration);
    game.get_camera()->set_tint_color(lerp(old_color, new_color, alpha));
}

bool Tint_Screen_Command::is_complete() const {
    return complete;
}