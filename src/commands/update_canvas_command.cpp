#include "../../include/commands/update_canvas_command.hpp"
#include "../../include/game.hpp"
#include "../../include/canvas.hpp"
#include "../../include/utility.hpp"

Update_Canvas_Command::Update_Canvas_Command(Game& game, Canvas& canvas,
	long duration, xd::vec2 pos, xd::vec2 mag, float angle, float opacity) :
	game(game),
	canvas(canvas),
	old_position(canvas.get_position()),
	old_magnification(canvas.get_magnification()),
	old_angle(static_cast<float>(canvas.get_angle())),
	old_opacity(canvas.get_opacity()),
	new_position(pos),
	new_magnification(mag),
	new_angle(angle),
	new_opacity(opacity),
	start_time(game.ticks()),
	duration(duration) {}

void Update_Canvas_Command::execute() {
	float alpha = calculate_alpha(game.ticks(), start_time, duration);
	canvas.set_position(lerp(old_position, new_position, alpha));
	canvas.set_magnification(lerp(old_magnification, new_magnification, alpha));
	canvas.set_angle(lerp(old_angle, new_angle, alpha));
	canvas.set_opacity(lerp(old_opacity, new_opacity, alpha));
}

bool Update_Canvas_Command::is_complete() const {
	bool complete = stopped || game.ticks() - start_time > duration;
	if (complete) {
		canvas.set_position(new_position);
		canvas.set_magnification(new_magnification);
		canvas.set_angle(new_angle);
		canvas.set_opacity(new_opacity);
	}
	return complete;
}

