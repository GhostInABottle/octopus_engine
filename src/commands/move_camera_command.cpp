#include "../../include/commands/move_camera_command.hpp"
#include "../../include/camera.hpp"
#include "../../include/map_object.hpp"
#include "../../include/utility/direction.hpp"


Move_Camera_Command::Move_Camera_Command(Camera& camera, float x, float y, float speed)
    : camera(camera), camera_object(camera.get_object()), speed(speed) {
    camera.set_object(nullptr);
    xd::vec2 displacement = camera.get_bounded_position(xd::vec2(x, y)) - camera.get_position();
    direction = xd::normalize(displacement);
    pixels = static_cast<float>(xd::length(displacement));
}

Move_Camera_Command::Move_Camera_Command(Camera& camera, Direction dir, float pixels, float speed)
    : camera(camera), camera_object(camera.get_object()), pixels(pixels), speed(speed) {
    camera.set_object(nullptr);
    direction = direction_to_vector(dir);
}

void Move_Camera_Command::execute() {
    do {
        xd::vec2 new_position = camera.get_position() + direction * speed;
        camera.set_position(new_position);
        pixels -= speed;
    } while (stopped && !is_complete());
}

bool Move_Camera_Command::is_complete() const {
    return pixels < 0.01f;
}