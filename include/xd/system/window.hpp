#ifndef H_XD_SYSTEM_WINDOW
#define H_XD_SYSTEM_WINDOW

#include "window_options.hpp"
#include "input.hpp"
#include "../graphics/transform_geometry.hpp"
#include "../event_bus.hpp"
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>

struct GLFWwindow;

namespace xd
{
    class window : public boost::noncopyable
    {
    public:
        // typedefs
        typedef xd::event_bus<input_args>::callback_t input_event_callback_t;
        typedef std::function<void ()> tick_callback_t;

        // public interface
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

        // ticks stuff
        int ticks() const;
        int delta_ticks() const;
        float delta_time() const;
        void register_tick_handler(tick_callback_t callback, boost::uint32_t interval);
        void unregister_tick_handler();
        int fps() const;
        int frame_count() const;

        void bind_key(const key& physical_key, const std::string& virtual_key);
        void unbind_key(const key& key);
        void unbind_key(const std::string& key);

        bool pressed(const key& key, int modifiers = 0) const;
        bool pressed(const std::string& key, int modifiers = 0) const;

        bool triggered(const key& key, int modifiers = 0) const;
        bool triggered(const std::string& key, int modifiers = 0) const;
        bool triggered_once(const key& key, int modifiers = 0);
        bool triggered_once(const std::string& key, int modifiers = 0);

        bool modifier(int modifiers) const;

        bool joystick_present(int id) const;

        event_link bind_input_event(const std::string& event_name, input_event_callback_t callback,
            const input_filter& filter = input_filter(), event_placement place = EVENT_PREPEND);
        void unbind_input_event(const std::string& event_name, event_link link);

        // an utility template function for member function callbacks, so user doesn't have to use std::bind directly
        template <typename T>
        event_link bind_input_event(const std::string& event_name, bool (T::*callback)(const input_args&), T* instance,
            const input_filter& filter = input_filter(), event_placement place = EVENT_PREPEND)
        {
            return bind_input_event(event_name, std::bind(callback, instance, std::placeholders::_1), filter, place);
        }

        // input event handler, for internal use
        void on_input(input_type type, int key, int action);

    private:
        GLFWwindow* m_window;
        // window width/height
        int m_width;
        int m_height;

        // keep track of ticks
        boost::uint32_t m_current_ticks;
        boost::uint32_t m_last_ticks;
        boost::uint32_t m_tick_handler_counter;
        boost::uint32_t m_tick_handler_interval;
        tick_callback_t m_tick_handler;

        // fps stuff
        int m_fps;
        int m_frame_count;
        boost::uint32_t m_last_fps_update;

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

        struct {
            float axes_values[7];
            unsigned char buttons[18];
            unsigned char prev_buttons[18];
        } joystick_state;


        // to keep track whether we're in update or not
        bool m_in_update;

        // event busses
        event_bus<input_args> m_input_events;
    };
}

#endif
