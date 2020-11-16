#include "../../include/commands/update_layer_command.hpp"
#include "../../include/game.hpp"
#include "../../include/layer.hpp"
#include "../../include/utility/math.hpp"

Update_Layer_Command::Update_Layer_Command(Game& game, Layer& layer, float opacity, long duration) :
    Timed_Command(game, duration),
    layer(layer),
    old_opacity(layer.opacity),
    new_opacity(opacity),
    complete(false) {}

void Update_Layer_Command::execute() {
    complete = stopped || is_done();
    layer.opacity = lerp(old_opacity, new_opacity, get_alpha(complete));
}

bool Update_Layer_Command::is_complete() const {
    return complete;
}