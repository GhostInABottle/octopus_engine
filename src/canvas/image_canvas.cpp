#include "../../include/canvas/image_canvas.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/utility/file.hpp"
#include "../../include/camera.hpp"
#include "../../include/exceptions.hpp"
#include "../../include/xd/graphics/sprite_batch.hpp"
#include <stdexcept>
#include <istream>

Image_Canvas::Image_Canvas(Game& game, const std::string& filename, xd::vec2 position, xd::vec4 trans)
    : Base_Image_Canvas(game, Base_Canvas::Type::IMAGE, position, filename)  {
    set_image(filename, trans);
}

void Image_Canvas::set_image(std::string image_filename, xd::vec4 trans) {
    string_utilities::normalize_slashes(image_filename);
    if (image_texture && filename == image_filename) return;

    auto fs = file_utilities::game_data_filesystem();
    auto stream = fs->open_binary_ifstream(image_filename);
    if (!stream || !*stream) {
        throw file_loading_exception("Failed to load image canvas: " + image_filename);
    }

    image_texture = std::make_shared<xd::texture>(image_filename, *stream, trans);
    filename = image_filename;

    redraw();
}

void Image_Canvas::render(Camera& camera, xd::sprite_batch& batch, Base_Canvas* parent) {
    xd::vec2 pos = get_position();
    if (is_camera_relative()) {
        auto camera_pos = camera.get_pixel_position();
        pos += camera_pos;
    }
    if (parent) {
        pos += parent->get_position();
    }

    auto angle = get_angle().value_or(0.0f);
    auto origin = get_origin().value_or(xd::vec2{ 0.5f, 0.5f });
    xd::vec2 mag = get_magnification();
    batch.add(image_texture, pos.x, pos.y, xd::radians(angle),
        mag, get_color(), origin);
}
