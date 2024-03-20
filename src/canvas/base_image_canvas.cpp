#include "../../include/canvas/base_image_canvas.hpp"

Base_Image_Canvas::Base_Image_Canvas(Game& game, Base_Canvas::Type type, xd::vec2 position, const std::string& filename)
    : Base_Canvas(game, type, position),
    filename(filename),
    magnification(1.0f, 1.0f),
    outline_color(std::nullopt)
{}


void Base_Image_Canvas::inherit_properties(const Base_Canvas& parent) {
    Base_Canvas::inherit_properties(parent);

    auto image_parent = dynamic_cast<const Base_Image_Canvas*>(&parent);
    if (!image_parent) return;

    set_angle(image_parent->get_angle());
    set_magnification(image_parent->get_magnification());
    set_origin(image_parent->get_origin());
}