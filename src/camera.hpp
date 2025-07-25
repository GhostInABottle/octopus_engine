#ifndef HPP_CAMERA
#define HPP_CAMERA

#include "xd/entity.hpp"
#include "xd/graphics/transform_geometry.hpp"
#include "xd/graphics/types.hpp"
#include <memory>
#include <optional>
#include <string>

class Game;
class Map_Object;
class Screen_Shaker;

class Camera : public xd::entity<Camera> {
public:
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    explicit Camera(Game& game, const std::string& default_scale_mode);
    ~Camera();
    // Called when game window size is changed
    void set_size(int width, int height, bool force = false);
    // Calculate viewport rectangle based on screen width and height
    xd::rect calculate_viewport(int width, int height);
    // Get clear color
    xd::vec4 get_clear_color() const {
        return current_clear_color;
    }
    // Set clear color
    void set_clear_color(std::optional<xd::vec4> color = std::nullopt);
    // Clear the screen to the clear color
    void clear_color_buffer() const;
    // Clear the depth buffer
    void clear_depth_buffer() const;
    // Clear both color and depth buffers
    void clear() const;
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
    void draw_rect(xd::rect rect, xd::vec4 color, bool fill = true) const;
    // Tint the screen with the map tint color
    void draw_map_tint() const;
    // Enable scissor test to limit drawing to a certain rectangle (game coords)
    void enable_scissor_test(xd::rect rect, xd::rect custom_viewport = xd::rect());
    // Disable scissor test
    void disable_scissor_test();
    // Apply a certain shader
    void set_shader(const std::string& vertex, const std::string& fragment);
    // Render current shader
    void render_shader();
    // Start shaking screen
    void start_shaking(xd::vec2 strength, xd::vec2 speed);
    // Cease shaking screen
    void cease_shaking();
    // Set shake offset
    void set_shake_offset(xd::vec2 shake_offset) {
        update_viewport(shake_offset);
    }
    // Getters and setters
    xd::vec2 get_position() const {
        return position;
    }
    xd::vec2 get_pixel_position() const {
        return xd::round(position);
    }
    xd::rect get_viewport() const {
        return viewport;
    }
    void set_viewport(xd::rect new_viewport) {
        viewport = new_viewport;
        update_viewport();
    }
    void use_calculated_viewport() {
        set_viewport(calculated_viewport);
    }
    xd::transform_geometry& get_geometry() {
        return geometry;
    }
    // Modelview projection matrix
    xd::mat4 get_mvp() const {
        return geometry.mvp();
    }
    xd::vec4 get_screen_tint() const {
        return screen_tint;
    }
    void set_screen_tint(xd::vec4 color) {
        screen_tint = color;
    }
    xd::vec4 get_map_tint() const {
        return map_tint;
    }
    void set_map_tint(xd::vec4 color) {
        map_tint = color;
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
    float get_saturation() const {
        return saturation;
    }
    void set_saturation(float saturation) {
        this->saturation = saturation;
    }
    const Map_Object* get_object() const {
        return object;
    }
    void set_object(Map_Object* obj) {
        object = obj;
    }
    xd::vec2 get_object_center_offset() const {
        return object_center_offset;
    }
    void set_object_center_offset(xd::vec2 offset) {
        object_center_offset = offset;
    }
    // Is the camera shaking?
    bool is_shaking() const {
        return shaker != nullptr;
    }
    // Get current shake offset
    xd::vec2 shake_offset() const;
private:
    // Setup viewport for rendering
    void update_viewport(xd::vec2 shake_offset = xd::vec2{ 0.0f }) const;
    // Game instance
    Game& game;
    // Camera position
    xd::vec2 position;
    // Current viewport rectangle
    xd::rect viewport;
    // Last calculated viewport
    xd::rect calculated_viewport;
    // Projection and model view matrices
    xd::transform_geometry geometry;
    // Screen tint color (affects everything)
    xd::vec4 screen_tint;
    // Map tint color (doesn't apply to canvases)
    xd::vec4 map_tint;
    // Screen brightness (-1 to 1, defaults to 0)
    float brightness;
    // Screen contrast (0+, defaults to 1)
    float contrast;
    // Screen color saturation (0+, defaults to 1)
    float saturation;
    // Tracked map object
    Map_Object* object;
    // Offset added when centering camera on objects
    xd::vec2 object_center_offset;
    // Screen shaker component
    std::shared_ptr<Screen_Shaker> shaker;
    // Current screen clearing color
    xd::vec4 current_clear_color;
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
    Screen_Shaker(xd::vec2 strength, xd::vec2 speed);
    void change_settings(xd::vec2 new_strength, xd::vec2 new_speed);
    xd::vec2 shake_offset() { return offset; }
    void update(Camera& camera);
private:
    // Shake strength
    xd::vec2 strength;
    // Shake speed
    xd::vec2 speed;
    // Last shake direction (-1 left or up, 1 right or down)
    xd::vec2 last_direction;
    // Shake offset
    xd::vec2 offset;
};

#endif
