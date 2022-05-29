#ifndef H_XD_SYSTEM_OPTIONS
#define H_XD_SYSTEM_OPTIONS

namespace xd
{
    struct window_options
    {
        window_options() noexcept
            : fullscreen(false)
            , default_windowed_width(800)
            , default_windowed_height(600)
            , allow_resize(false)
            , display_cursor(true)
            , vsync(true)
            , enable_joystick(true)
            , gamepad_detection(true)
            , axis_as_dpad(false)
            , stick_sensitivity(0.5f)
            , trigger_sensitivity(0.5f)
            , depth_bits(8)
            , stencil_bits(0)
            , antialiasing_level(0)
            , major_version(2)
            , minor_version(0)
        {}

        window_options(bool fullscreen, int window_width, int window_height,
            bool allow_resize, bool display_cursor, bool vsync,
            bool enable_joystick, bool gamepad_detection, bool axis_as_dpad, float stick_sensitivity,
            float trigger_sensitivity, int depth_bits, int stencil_bits, int antialiasing_level,
            int major_version, int minor_version) noexcept
            : fullscreen(fullscreen)
            , default_windowed_width(window_width)
            , default_windowed_height(window_height)
            , allow_resize(allow_resize)
            , display_cursor(display_cursor)
            , vsync(vsync)
            , enable_joystick(enable_joystick)
            , gamepad_detection(gamepad_detection)
            , axis_as_dpad(axis_as_dpad)
            , stick_sensitivity(stick_sensitivity)
            , trigger_sensitivity(trigger_sensitivity)
            , depth_bits(depth_bits)
            , stencil_bits(stencil_bits)
            , antialiasing_level(antialiasing_level)
            , major_version(major_version)
            , minor_version(minor_version)
        {}

        bool fullscreen;
        int default_windowed_width;
        int default_windowed_height;
        bool allow_resize;
        bool display_cursor;
        bool vsync;
        bool enable_joystick;
        bool gamepad_detection;
        bool axis_as_dpad;
        float stick_sensitivity;
        float trigger_sensitivity;
        int depth_bits;
        int stencil_bits;
        int antialiasing_level;
        int major_version;
        int minor_version;
    };
}

#endif
