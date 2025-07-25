#include "update_opacity_command.hpp"
#include "../game.hpp"
#include "../interfaces/opacity_holder.hpp"
#include "../utility/math.hpp"

Update_Opacity_Command::Update_Opacity_Command(Game& game, Opacity_Holder& holder, float opacity, long duration)
        : Timed_Command(game, duration)
        , opacity_holder(holder)
        , old_opacity(holder.get_opacity())
        , new_opacity(opacity)
        , complete(false) {
    map_ptr = game.get_map();
}

void Update_Opacity_Command::execute() {
    if (complete) return;

    complete = stopped || is_done();

    auto opacity = lerp(old_opacity, new_opacity, get_alpha(complete));
    opacity_holder.set_opacity(opacity);
}

bool Update_Opacity_Command::is_complete() const {
    return complete || force_stopped;
}

void Update_Opacity_Command::restart(float opacity, long duration) {
    Timed_Command::reset(duration);
    old_opacity = opacity_holder.get_opacity();
    new_opacity = opacity;
    complete = false;
}
