#ifndef H_XD_SYSTEM_WINDOW
#define H_XD_SYSTEM_WINDOW

#include "window_options.hpp"
#include "input.hpp"
#include "../graphics/transform_geometry.hpp"
#include "..//graphics/types.hpp"
#include "../event_bus.hpp"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace xd
{
    class window
    {
    public:
        // typedefs
        typedef xd::event_bus<input_args>::callback_t input_event_callback_t;
        typedef std::function<void ()> tick_callback_t;

        // public interface
        window(const window&) = delete;
        window& operator=(const window&) = delete;
        window(const std::string& title, int width, int height, const window_options& options = window_options());
        virtual ~window();

        void update();
        void clear();
        void swap();

        bool focused() const;
        bool closed() const;
        int width() const;
        int height() const;
        int framebuffer_width() const;
        int framebuffer_height() const;
        void set_size(int width, int height);
        xd::vec2 get_size() const;
        std::vector<xd::vec2> get_sizes() const;
        bool is_fullscreen() const;
        void set_fullscreen(bool fullscreen);

        // ticks stuff
        int ticks() const { return m_current_ticks; }
        int delta_ticks() const { return m_current_ticks - m_last_ticks; }
        float delta_time() const {
            return static_cast<float>(delta_ticks()) / 1000.0f;
        }
        void register_tick_handler(tick_callback_t callback, std::uint32_t interval) {
            m_tick_handler = callback;
            m_tick_handler_interval = interval;
            m_tick_handler_counter = 0;
        }
        void unregister_tick_handler() {
            m_tick_handler = nullptr;
        }
        int fps() const { return m_fps; }
        int frame_count() const { return m_frame_count; }

        void bind_key(const key& physical_key, const std::string& virtual_key);
        void unbind_key(const key& key);
        void unbind_key(const std::string& key);

        bool pressed(const key& key, int joystick_id = -1) const;
        bool pressed(const std::string& key, int joystick_id = -1) const;

        bool triggered() const {
            auto& key_set = m_in_update ? m_tick_handler_triggered_keys : m_triggered_keys;
            return !key_set.empty();
        }
        bool triggered(const key& key, int joystick_id = -1) const;
        bool triggered(const std::string& key, int joystick_id = -1) const;
        bool triggered_once(const key& key, int joystick_id = -1);
        bool triggered_once(const std::string& key, int joystick_id = -1);
        std::unordered_set<xd::key> triggered_keys() const {
            return m_in_update ? m_tick_handler_triggered_keys : m_triggered_keys;
        }

        float axis_value(const key& key, int joystick_id = -1);
        float axis_value(const std::string& key, int joystick_id = -1);

        bool is_joystick_enabled() const {
            return m_joystick_enabled;
        }
        void set_joystick_enabled(bool enabled) {
            m_joystick_enabled = enabled;
        }
        bool joystick_present(int id) const {
            return m_joystick_states.find(id) != m_joystick_states.end();
        }
        bool joystick_is_gamepad(int id) const;
        void add_joystick(int id);
        void remove_joystick(int id);
        int first_joystick_id() const;
        std::unordered_map<int, std::string> joystick_names() const;

        event_link bind_input_event(const std::string& event_name, input_event_callback_t callback,
                const input_filter& filter = input_filter(), event_placement place = event_placement::EVENT_PREPEND) {
            return m_input_events[event_name].add(callback, filter, place);
        }
        void unbind_input_event(const std::string& event_name, event_link link) {
            m_input_events[event_name].remove(link);
        }

        // an utility template function for member function callbacks, so user doesn't have to use std::bind directly
        template <typename T>
        event_link bind_input_event(const std::string& event_name, bool (T::*callback)(const input_args&), T* instance,
            const input_filter& filter = input_filter(), event_placement place = EVENT_PREPEND)
        {
            return bind_input_event(event_name, std::bind(callback, instance, std::placeholders::_1), filter, place);
        }

        // input event handler, for internal use
        void on_input(input_type type, int key, int action, int device_id = 0);

    private:
        GLFWwindow* m_window;
        // window width/height
        int m_width;
        int m_height;
        // windowed mode position
        xd::ivec2 m_windowed_pos;
        // windowed mode content size
        xd::ivec2 m_windowed_size;

        bool m_joystick_enabled;
        bool m_gamepad_detection;
        bool m_axis_as_dpad;
        float m_axis_sensitivity;

        // keep track of ticks
        std::uint32_t m_current_ticks;
        std::uint32_t m_last_ticks;
        std::uint32_t m_tick_handler_counter;
        std::uint32_t m_tick_handler_interval;
        tick_callback_t m_tick_handler;

        // fps stuff
        int m_fps;
        int m_frame_count;
        std::uint32_t m_last_fps_update;

        // internal typedefs
        typedef std::unordered_set<key> key_set_t;
        typedef std::unordered_map<key, std::string> key_table_t;
        typedef std::unordered_map<std::string, key_set_t> virtual_table_t;
        typedef std::unordered_set<key> trigger_keys_t;

        // virtual key conversions
        key_table_t m_key_to_virtual;
        virtual_table_t m_virtual_to_key;

        // key triggers
        trigger_keys_t m_triggered_keys;
        trigger_keys_t m_tick_handler_triggered_keys;

        // joystick/gamepad state
        struct joystick_state {
            float axes[6];
            unsigned char buttons[15];
            unsigned char prev_buttons[15];
        };

        std::unordered_map<int, joystick_state> m_joystick_states;

        // to keep track whether we're in update or not
        bool m_in_update;

        // event busses
        event_bus<input_args> m_input_events;

        // private functions
        void update_joysticks();
    };
}

#endif
