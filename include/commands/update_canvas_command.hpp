#ifndef HPP_UPDATE_CANVAS_COMMAND
#define HPP_UPDATE_CANVAS_COMMAND

#include <xd/graphics/types.hpp>
#include "../command.hpp"

class Game;
class Canvas;

class Update_Canvas_Command : public Command {
public:
    Update_Canvas_Command(Game& game, Canvas& canvas);
	Update_Canvas_Command(Game& game, Canvas& canvas, long duration,
		xd::vec2 pos, xd::vec2 mag, float angle, float opacity);
    void reset(bool reset_new = true);
    void set_new_position(xd::vec2 position) { new_position = position;  }
    void set_new_magnification(xd::vec2 magnification) { new_magnification = magnification;  }
    void set_new_angle(float angle) { new_angle = angle;  }
    void set_new_opacity(float opacity) { new_opacity = opacity;  }
    void set_duration(long ms) { duration = ms; }
	void execute();
	bool is_complete() const;
private:
    void update_canvas(float alpha) const;
	Game& game;
	Canvas& canvas;
	xd::vec2 old_position;
	xd::vec2 old_magnification;
	float old_angle;
	float old_opacity;
	xd::vec2 new_position;
	xd::vec2 new_magnification;
	float new_angle;
	float new_opacity;
	long start_time;
	long duration;
};


#endif
