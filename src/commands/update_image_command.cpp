#include "../../include/canvas/base_image_canvas.hpp"
#include "../../include/commands/update_image_command.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/math.hpp"

Update_Image_Command::Update_Image_Command(Game& game, Base_Image_Canvas& canvas)
        : Timed_Command(game, 0), canvas(canvas) {
    map_ptr = game.get_map();
    reset();
}

Update_Image_Command::Update_Image_Command(Game& game, Base_Image_Canvas& canvas,
    int duration, Parameters parameters)
        : Timed_Command(game, duration)
        , canvas(canvas)
        , new_parameters(parameters) {
    map_ptr = game.get_map();
    reset(false);
}

void Update_Image_Command::reset(bool reset_new) {
    old_parameters = Parameters(canvas);

    if (reset_new) {
        new_parameters = old_parameters;
    }

    start_time = game.ticks();
    stopped = false;
    complete = false;
}

void Update_Image_Command::execute() {
    if (complete) return;

    complete = stopped || is_done();
    update_canvas(get_alpha(complete));
}

bool Update_Image_Command::is_complete() const {
    return complete || force_stopped;
}

void Update_Image_Command::update_canvas(float alpha) const {
    if (new_parameters.position != old_parameters.position) {
        auto position = lerp(old_parameters.position, new_parameters.position, alpha);
        canvas.set_position(position);
    }

    if (new_parameters.magnification != old_parameters.magnification) {
        auto magnification = lerp(old_parameters.magnification,
            new_parameters.magnification, alpha);
        canvas.set_magnification(magnification);
    }

    auto& new_angle = new_parameters.angle;
    if (new_angle.has_value() && new_angle != old_parameters.angle) {
        auto angle = lerp(old_parameters.angle.value_or(0), new_angle.value(), alpha);
        canvas.set_angle(angle);
    }

    if (new_parameters.opacity != old_parameters.opacity) {
        auto opacity = lerp(old_parameters.opacity, new_parameters.opacity, alpha);
        canvas.set_opacity(opacity);
    }

    canvas.redraw();
}
