#include "../../include/commands/update_image_command.hpp"
#include "../../include/game.hpp"
#include "../../include/canvas/base_image_canvas.hpp"
#include "../../include/utility/math.hpp"

Update_Image_Command::Update_Image_Command(Game& game, Base_Image_Canvas& canvas) :
        Timed_Command(game, 0), canvas(canvas) {
    map_ptr = game.get_map();
    reset();
}

Update_Image_Command::Update_Image_Command(Game& game, Base_Image_Canvas& canvas,
            long duration, xd::vec2 pos, xd::vec2 mag,
            std::optional<float> angle, float opacity)
        : Timed_Command(game, duration),
        canvas(canvas),
        new_position(pos),
        new_magnification(mag),
        new_angle(angle),
        new_opacity(opacity) {
    map_ptr = game.get_map();
    reset(false);
}

void Update_Image_Command::reset(bool reset_new) {
    old_position = canvas.get_position();
    old_magnification = canvas.get_magnification();
    old_angle = canvas.get_angle();
    old_opacity = canvas.get_opacity();

    if (reset_new) {
        new_position = old_position;
        new_magnification = old_magnification;
        new_angle = old_angle;
        new_opacity = old_opacity;
    }

    start_time = game.ticks();
    stopped = false;
    complete = false;
}

void Update_Image_Command::set_duration(int ms) {
    duration = ms;
}

void Update_Image_Command::execute() {
    if (complete) return;

    complete = stopped || is_done();
    update_canvas(get_alpha(complete));
}

bool Update_Image_Command::is_complete() const {
    return complete;
}

void Update_Image_Command::update_canvas(float alpha) const {
    if (new_position != old_position)
        canvas.set_position(lerp(old_position, new_position, alpha));

    if (new_magnification != old_magnification)
        canvas.set_magnification(lerp(old_magnification, new_magnification, alpha));

    if (new_angle.has_value() && new_angle != old_angle)
        canvas.set_angle(lerp(old_angle.value_or(0), new_angle.value(), alpha));

    if (new_opacity != old_opacity)
        canvas.set_opacity(lerp(old_opacity, new_opacity, alpha));

    canvas.redraw();
}
