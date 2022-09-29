#ifndef HPP_BASE_IMAGE_CANVAS
#define HPP_BASE_IMAGE_CANVAS

#include "base_canvas.hpp"
#include <optional>

// Base class for image and sprite canvases
class Base_Image_Canvas : public Base_Canvas {
public:
    Base_Image_Canvas(const Base_Image_Canvas&) = delete;
    Base_Image_Canvas& operator=(const Base_Image_Canvas&) = delete;
    // Inherit certain properties from another canvas
    void inherit_properties(const Base_Canvas& parent) override;
    // Getters and setters
    std::optional<xd::vec2> get_origin() const {
        return origin;
    }
    void set_origin(std::optional<xd::vec2> new_origin) {
        if (origin == new_origin)
            return;
        origin = new_origin;
        redraw();
    }
    xd::vec2 get_magnification() const {
        return magnification;
    }
    void set_magnification(xd::vec2 new_magnification) {
        if (magnification == new_magnification)
            return;
        magnification = new_magnification;
        redraw();
    }
    std::optional<float> get_angle() const {
        return angle;
    }
    void set_angle(std::optional<float> new_angle) {
        if (angle == new_angle)
            return;
        angle = new_angle;
        redraw();
    }
    std::string get_filename() const {
        return filename;
    }
    std::optional<xd::vec4> get_outline_color() const {
        return outline_color;
    }
    void set_outline_color(std::optional<xd::vec4> color) {
        outline_color = color;
    }
protected:
    // Sets shared default values
    Base_Image_Canvas(Game& game, Base_Canvas::Type type, xd::vec2 position, const std::string& filename);
private:
    // Drawing origin
    std::optional<xd::vec2> origin;
    // X and Y magnification
    xd::vec2 magnification;
    // Rotation angle in degrees
    std::optional<float> angle;
    // Image filename
    std::string filename;
    // Color of image outline
    std::optional<xd::vec4> outline_color;
};

#endif

