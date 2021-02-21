#ifndef HPP_UPDATE_CANVAS_COMMAND
#define HPP_UPDATE_CANVAS_COMMAND

#include<optional>
#include "../xd/graphics/types.hpp"
#include "timed_command.hpp"

class Game;
class Canvas;

class Update_Canvas_Command : public Timed_Command {
public:
    Update_Canvas_Command(Game& game, Canvas& canvas);
    Update_Canvas_Command(Game& game, Canvas& canvas, long duration,
        xd::vec2 pos, xd::vec2 mag, std::optional<float> angle, float opacity);
    void reset(bool reset_new = true);
    void set_new_position(xd::vec2 position) { new_position = position;  }
    void set_new_magnification(xd::vec2 magnification) { new_magnification = magnification;  }
    void set_new_angle(std::optional<float> angle) { new_angle = angle;  }
    void set_new_opacity(float opacity) { new_opacity = opacity;  }
    void set_duration(int ms);
    void execute() override;
    bool is_complete() const override;
private:
    void update_canvas(float alpha) const;
    Canvas& canvas;
    xd::vec2 old_position;
    xd::vec2 old_magnification;
    std::optional<float> old_angle;
    float old_opacity;
    xd::vec2 new_position;
    xd::vec2 new_magnification;
    std::optional<float> new_angle;
    float new_opacity;
    bool complete;
};


#endif
