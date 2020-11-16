#ifndef HPP_UPDATE_LAYER_COMMAND
#define HPP_UPDATE_LAYER_COMMAND

#include "timed_command.hpp"

class Game;
struct Layer;

class Update_Layer_Command : public Timed_Command {
public:
    Update_Layer_Command(Game& game, Layer& layer, float opacity, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Layer& layer;
    float old_opacity;
    float new_opacity;
    bool complete;
};


#endif
