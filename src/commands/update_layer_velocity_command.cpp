#include "../../include/commands/update_layer_velocity_command.hpp"
#include "../../include/game.hpp"
#include "../../include/layers/image_layer.hpp"
#include "../../include/utility/math.hpp"

Update_Layer_Velocity_Command::Update_Layer_Velocity_Command(Game& game, Image_Layer& layer, xd::vec2 velocity, long duration)
    : Timed_Command(game, duration)
    , layer(layer)
    , old_velocity(layer.get_velocity())
    , new_velocity(velocity)
    , complete(false) {
    map_ptr = game.get_map();
}

void Update_Layer_Velocity_Command::execute() {
    if (complete) return;

    complete = stopped || is_done();

    auto velocity = lerp(old_velocity, new_velocity, get_alpha(complete));
    layer.set_velocity(velocity);
}

bool Update_Layer_Velocity_Command::is_complete() const {
    return complete || force_stopped;
}
