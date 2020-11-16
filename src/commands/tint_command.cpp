#include "../../include/commands/tint_command.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/utility/math.hpp"

Tint_Command::Tint_Command(Tint_Target target, Game& game, xd::vec4 color, long duration) :
        Timed_Command(game, duration), target(target), new_color(color), complete(false) {
    auto camera = game.get_camera();
    old_color = target == Tint_Target::MAP
        ? camera->get_map_tint()
        : camera->get_screen_tint();
}

void Tint_Command::execute() {
    complete = stopped || is_done();
    float alpha = get_alpha(complete);
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