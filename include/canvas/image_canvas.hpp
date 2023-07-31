#ifndef HPP_IMAGE_CANVAS
#define HPP_IMAGE_CANVAS

#include "base_image_canvas.hpp"

// A canvas based on an image file
class Image_Canvas : public Base_Image_Canvas {
public:
    Image_Canvas(const Image_Canvas&) = delete;
    Image_Canvas& operator=(const Image_Canvas&) = delete;
    // Create an image canvas with an optional transparent color
    Image_Canvas(Game& game, const std::string& filename, xd::vec2 position, xd::vec4 trans = xd::vec4(0));
    // Render the image
    void render(Camera& camera, xd::sprite_batch& batch, Base_Canvas* parent) override;
    // Change the image
    void set_image(std::string image_filename, xd::vec4 trans);
    // Get the image texture
    std::shared_ptr<xd::texture> get_image_texture() const {
        return image_texture;
    }
    // Get canvas width
    int get_width() const {
        return image_texture->width();
    }
    // Get canvas height
    int get_height() const {
        return image_texture->height();
    }
private:
    // Image texture
    std::shared_ptr<xd::texture> image_texture;
};

#endif