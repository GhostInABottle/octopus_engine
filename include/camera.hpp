#ifndef HPP_CAMERA
#define HPP_CAMERA

#include <memory>
#include "xd/system.hpp"
#include "xd/graphics/types.hpp"
#include "xd/entity.hpp"

class Game;
class Map_Object;
class Screen_Shaker;

class Camera : public xd::entity<Camera> {
public:
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    explicit Camera(Game& game);
    // Called when game window size is changed
    void set_size(int width, int height);
    // Calculate viewport rectangle based on screen width and height
    void calculate_viewport(int width, int height);
    // Setup viewport for rendering
    void update_viewport(float shake_offset = 0.0f) const;
    // Setup initial OpenGL state
    void setup_opengl() const;
    // Set OpenGL clear color
    void set_clear_color(xd::vec4 color) const;
    // Center camera at position
    void center_at(xd::vec2 pos);
    // Center camera at object
    void center_at(const Map_Object& object);
    // Get camera centered position around pos
    xd::vec2 get_centered_position(xd::vec2 pos) const;
    // Get camera centered position around object
    xd::vec2 get_centered_position(const Map_Object& object) const;
    // Get minimum and maximum position bounds to stay within map limits
    xd::rect get_position_bounds() const;
    // Get position within camera bounds
    xd::vec2 get_bounded_position(xd::vec2 pos) const;
    // Update position within map bounds
    void set_position(xd::vec2 pos);
    // Draw a rectangle
    void draw_rect(xd::rect rect, xd::vec4 color, bool fill = true);
    // Enable scissor test to limit drawing to a certain rectangle (game coords)
    void enable_scissor_test(xd::rect rect, xd::rect custom_viewport = xd::rect());
    // Disable scissor test
    void disable_scissor_test();
    // Apply a certain shader
    void set_shader(const std::string& vertex, const std::string& fragment);
    // Render current shader
    void render_shader();
    // Start shaking screen
    void start_shaking(float strength, float speed);
    // Cease shaking screen
    void cease_shaking();
    // Getters and setters
    xd::vec2 get_position() const {
        return position;
    }
    xd::rect get_viewport() const {
        return viewport;
    }
    xd::transform_geometry& get_geometry() {
        return geometry;
    }
    // Modelview projection matrix
    xd::mat4 get_mvp() const {
        return geometry.mvp();
    }
    xd::vec4 get_tint_color() const {
        return tint_color;
    }
    void set_tint_color(xd::vec4 color) {
        tint_color = color;
    }
    float get_brightness() const {
        return brightness;
    }
    void set_brightness(float brightness) {
        this->brightness = brightness;
    }
    float get_contrast() const {
        return contrast;
    }
    void set_contrast(float contrast) {
        this->contrast = contrast;
    }
    const Map_Object* get_object() const {
        return object;
    }
    void set_object(Map_Object* obj) {
        object = obj;
    }
    // Is the camera shaking?
    bool is_shaking() const {
        return shaker != nullptr;
    }
    // Get current shake offset
    float shake_offset() const;
private:
    // Game instance
    Game& game;
    // Camera position
    xd::vec2 position;
    // Viewport rectangle
    xd::rect viewport;
    // Projection and model view matrices
    xd::transform_geometry geometry;
    // Screen tint color
    xd::vec4 tint_color;
    // Screen brightness
    float brightness;
    // Screen contrast
    float contrast;
    // Tracked map object
    Map_Object* object;
    // Screen shaker component
    std::shared_ptr<Screen_Shaker> shaker;
    // Implementation details
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

class Camera_Renderer : public xd::render_component<Camera> {
public:
    explicit Camera_Renderer(Game& game) : game(game) {}
    void render(Camera& camera);
private:
    Game& game;
};

class Object_Tracker : public xd::logic_component<Camera> {
public:
    void update(Camera& camera);
};

class Screen_Shaker : public xd::logic_component<Camera> {
public:
    Screen_Shaker(float str, float spd)
            : strength(str), speed(spd), direction(1), offset(0) {}
    float shake_offset() { return offset; }
    void update(Camera& camera);
private:
    // Shake strength
    float strength;
    // Shake speed
    float speed;
    // Shake direction (-1 left, 1 right)
    float direction;
    // Shake offset
    float offset;
};

#endif
