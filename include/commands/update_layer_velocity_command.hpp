#ifndef HPP_UPDATE_LAYER_VELOCITY_COMMAND
#define HPP_UPDATE_LAYER_VELOCITY_COMMAND

#include "timed_command.hpp"
#include "../xd/glm.hpp"

struct Image_Layer;

// Update the velocity of an image layer
class Update_Layer_Velocity_Command : public Timed_Command {
public:
    Update_Layer_Velocity_Command(Game& game, Image_Layer& layer, xd::vec2 velocity, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Image_Layer& layer;
    xd::vec2 old_velocity;
    xd::vec2 new_velocity;
    bool complete;
};

#endif