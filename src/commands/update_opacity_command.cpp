#include "../../include/commands/update_opacity_command.hpp"
#include "../../include/game.hpp"
#include "../../include/layer.hpp"
#include "../../include/utility/math.hpp"

Update_Opacity_Command::Update_Opacity_Command(Game& game, Translucent_Object& obj, float opacity, long duration)
    : Timed_Command(game, duration),
    translucent_object(obj),
    old_opacity(obj.get_opacity()),
    new_opacity(opacity),
    complete(false) {}

void Update_Opacity_Command::execute() {
    complete = stopped || game.is_paused() || is_done();
    auto opacity = lerp(old_opacity, new_opacity, get_alpha(complete));
    translucent_object.set_opacity(opacity);
}

bool Update_Opacity_Command::is_complete() const {
    return complete;
}

void Update_Opacity_Command::restart(float opacity, long duration) {
    Timed_Command::reset(duration);
    old_opacity = translucent_object.get_opacity();
    new_opacity = opacity;
    complete = false;
}