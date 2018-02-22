#include "../../include/commands/update_layer_command.hpp"
#include "../../include/game.hpp"
#include "../../include/layer.hpp"
#include "../../include/utility.hpp"

Update_Layer_Command::Update_Layer_Command(Game& game, Layer& layer, float opacity, long duration) :
    game(game),
    layer(layer),
    old_opacity(layer.opacity),
    new_opacity(opacity),
    start_time(game.ticks()),
    duration(duration) {}

void Update_Layer_Command::execute() {
    float alpha = calculate_alpha(game.ticks(), start_time, duration);
    layer.opacity = lerp(old_opacity, new_opacity, alpha);
}

bool Update_Layer_Command::is_complete() const {
    bool complete = stopped || game.ticks() - start_time > duration;
    if (complete)
        layer.opacity = new_opacity;
    return complete;
}