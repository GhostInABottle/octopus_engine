#include "../../include/commands/update_color_command.hpp"
#include "../../include/interfaces/color_holder.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/math.hpp"

Update_Color_Command::Update_Color_Command(Game& game, Color_Holder& holder, xd::vec4 color, long duration)
    : Timed_Command(game, duration)
    , color_holder(holder)
    , old_color(holder.get_color())
    , new_color(color)
    , complete(false) {
    map_ptr = game.get_map();
}

void Update_Color_Command::execute() {
    if (complete) return;

    complete = stopped || game.is_paused() || is_done();

    auto color = lerp(old_color, new_color, get_alpha(complete));
    color_holder.set_color(color);
}

bool Update_Color_Command::is_complete() const {
    return complete || force_stopped;
}
