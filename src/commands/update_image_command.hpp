#ifndef HPP_UPDATE_IMAGE_COMMAND
#define HPP_UPDATE_IMAGE_COMMAND

#include "../xd/glm.hpp"
#include "timed_command.hpp"
#include<optional>

class Game;
class Base_Image_Canvas;

class Update_Image_Command : public Timed_Command {
public:
    struct Parameters {
        xd::vec2 position;
        xd::vec2 magnification;
        std::optional<float> angle;
        float opacity;
        Parameters() : opacity(1.0f) {}
        Parameters(const Base_Image_Canvas& canvas);
    };
    Update_Image_Command(Game& game, Base_Image_Canvas& canvas);
    Update_Image_Command(Game& game, Base_Image_Canvas& canvas, int duration, Parameters parameters);
    void reset(bool reset_new = true);
    void execute() override;
    bool is_complete() const override;
private:
    void update_canvas(float alpha) const;
    Base_Image_Canvas& canvas;
    Parameters old_parameters;
    Parameters new_parameters;
    bool complete;
};

#endif
