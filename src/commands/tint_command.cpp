#include "../../include/commands/tint_command.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/utility/math.hpp"

Tint_Command::Tint_Command(Tint_Target target, Game& game, xd::vec4 color, long duration) :
        target(target), game(game), new_color(color),
        start_time(game.ticks()), duration(duration), complete(false) {
    auto camera = game.get_camera();
    old_color = target == Tint_Target::MAP
        ? camera->get_map_tint()
        : camera->get_screen_tint();
}

void Tint_Command::execute() {
    complete = stopped || game.ticks() - start_time > duration;
    float alpha = complete ? 1.0f : calculate_alpha(game.ticks(), start_time, duration);
    auto color = lerp(old_color, new_color, alpha);
    auto camera = game.get_camera();
    if (target == Tint_Target::MAP)
        camera->set_map_tint(color);
    else
        camera->set_screen_tint(color);
}

bool Tint_Command::is_complete() const {
    return complete;
}