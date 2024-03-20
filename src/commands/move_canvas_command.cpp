#include "../../include/canvas/base_canvas.hpp"
#include "../../include/commands/move_canvas_command.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/math.hpp"

Move_Canvas_Command::Move_Canvas_Command(Game& game, Base_Canvas& canvas, xd::vec2 pos, long duration)
        : Timed_Command(game, duration)
        , canvas(canvas)
        , old_position(canvas.get_position())
        , new_position(pos)
        , complete(false) {
    map_ptr = game.get_map();
}

void Move_Canvas_Command::execute() {
    complete = stopped || is_done();
    auto alpha = get_alpha(complete);
    if (new_position == old_position) return;

    canvas.set_position(lerp(old_position, new_position, alpha));
    canvas.redraw();
}

bool Move_Canvas_Command::is_complete() const {
    return complete || force_stopped;
}
