#ifndef HPP_UPDATE_OPACITY_COMMAND
#define HPP_UPDATE_OPACITY_COMMAND

#include "timed_command.hpp"

class Translucent_Object;

class Update_Opacity_Command : public Timed_Command {
public:
    Update_Opacity_Command(Game& game, Translucent_Object& translucent_object, float opacity, long duration);
    void execute() override;
    bool is_complete() const override;
    void restart(float opacity, long duration);
private:
    Translucent_Object& translucent_object;
    float old_opacity;
    float new_opacity;
    bool complete;
};

#endif
