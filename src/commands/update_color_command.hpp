#ifndef HPP_UPDATE_COLOR_COMMAND
#define HPP_UPDATE_COLOR_COMMAND

#include "timed_command.hpp"
#include "../xd/glm.hpp"

class Color_Holder;

// Update the color of a canvas, map object, or object/image layer
class Update_Color_Command : public Timed_Command {
public:
    Update_Color_Command(Game& game, Color_Holder& color_holder, xd::vec4 color, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Color_Holder& color_holder;
    xd::vec4 old_color;
    xd::vec4 new_color;
    bool complete;
};

#endif
