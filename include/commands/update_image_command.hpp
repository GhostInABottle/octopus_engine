#ifndef HPP_UPDATE_IMAGE_COMMAND
#define HPP_UPDATE_IMAGE_COMMAND

#include<optional>
#include "../xd/graphics/types.hpp"
#include "timed_command.hpp"

class Game;
class Base_Image_Canvas;

class Update_Image_Command : public Timed_Command {
public:
    Update_Image_Command(Game& game, Base_Image_Canvas& canvas);
    Update_Image_Command(Game& game, Base_Image_Canvas& canvas, long duration,
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
    Base_Image_Canvas& canvas;
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
