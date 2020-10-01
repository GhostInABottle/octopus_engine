#include "../../../include/xd/system/window.hpp"
#include "../../../include/xd/system/exceptions.hpp"
#include "../../../include/xd/vendor/glm/gtx/hash.hpp"
#include <GL/glew.h>
#ifdef _WIN32
    #include <GL/glfw3.h>
#else
    #include <GLFW/glfw3.h>
#endif
#include <cstdlib>

// detail stuff, hidden from user
namespace
{
    xd::window *window_instance = nullptr;

    void on_key_proxy(GLFWwindow*, int key, int, int action, int)
    {
        window_instance->on_input(xd::input_type::INPUT_KEYBOARD, key, action);
    }

    void on_mouse_proxy(GLFWwindow*, int key, int action, int)
    {
        window_instance->on_input(xd::input_type::INPUT_MOUSE, key, action);
    }

    void on_joystick_changed(int id, int event) {
        if (event == GLFW_CONNECTED) {
            window_instance->add_joystick(id);
        } else if (event == GLFW_DISCONNECTED) {
            window_instance->remove_joystick(id);
        }
    }
};

xd::window::window(const std::string& title, int width, int height, const window_options& options)
    : m_width(width)
    , m_height(height)
    , m_windowed_pos(0, 0)
    , m_windowed_size(0, 0)
    , m_in_update(false)
    , m_tick_handler_counter(0)
    , m_tick_handler_interval(0)
{
    // check if there's already a window alive
    if (window_instance) {
        throw xd::window_creation_failed();
    }

    // initialize glfw
    if (glfwInit() == GL_FALSE) {
        throw xd::window_creation_failed();
    }

    glfwWindowHint(GLFW_RESIZABLE, options.allow_resize);
    glfwWindowHint(GLFW_SAMPLES, options.antialiasing_level);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, options.major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, options.minor_version);
    glfwWindowHint(GLFW_DEPTH_BITS, options.depth_bits);
    glfwWindowHint(GLFW_STENCIL_BITS, options.stencil_bits);

    m_window = glfwCreateWindow(m_width, m_height, title.c_str(),
            options.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

    if (!m_window)
    {
        glfwTerminate();
        throw xd::window_creation_failed();
    }
    glfwMakeContextCurrent(m_window);

    if (options.display_cursor)
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_FALSE);
    glfwSetInputMode(m_window, GLFW_STICKY_MOUSE_BUTTONS, GL_FALSE);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        throw xd::window_creation_failed();
    }

    set_vsync(options.vsync);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    // intialize ticks
    m_current_ticks = m_last_ticks = m_last_fps_update = static_cast<std::uint32_t>(glfwGetTime() * 1000);
    m_fps = m_frame_count = 0;

    // register input callbacks
    glfwSetKeyCallback(m_window, &on_key_proxy);
    glfwSetMouseButtonCallback(m_window, &on_mouse_proxy);
    glfwSetJoystickCallback(&on_joystick_changed);

    window_instance = this;

    m_joystick_enabled = options.enable_joystick;
    m_gamepad_detection = options.gamepad_detection;
    m_axis_as_dpad = options.axis_as_dpad;
    m_axis_sensitivity = options.axis_sensitivity;

    for (int joystick = GLFW_JOYSTICK_1; joystick < GLFW_JOYSTICK_LAST + 1; ++joystick) {
        add_joystick(joystick);
    }
}

xd::window::~window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
    window_instance = nullptr;
}

bool xd::window::joystick_is_gamepad(int id) const {
    return glfwJoystickIsGamepad(id) != 0;
}

void xd::window::add_joystick(int id) {
    if (glfwJoystickPresent(id)) {
        for (int button = 0; button < GLFW_GAMEPAD_BUTTON_LAST + 1; ++button) {
            m_joystick_states[id].buttons[button] = GLFW_RELEASE;
        }
        for (int axis = 0; axis < GLFW_GAMEPAD_AXIS_LAST + 1; ++axis) {
            m_joystick_states[id].axes[axis] = 0.0f;
        }
    }
}

void xd::window::remove_joystick(int id) {
    if (joystick_present(id)) {
        m_joystick_states.erase(id);
    }
}

void xd::window::on_input(input_type type, int key, int action, int device_id)
{
    input_args args;

    // find associated virtual key
    xd::window::key_table_t::iterator i = m_key_to_virtual.find(args.physical_key);
    if (i != m_key_to_virtual.end()) {
        args.virtual_key = i->second;
    }

    switch (type) {
    case input_type::INPUT_KEYBOARD:
        args.physical_key = KEY(key);
        break;
    case input_type::INPUT_GAMEPAD:
        args.physical_key = GAMEPAD(key, device_id);
        break;
    case input_type::INPUT_MOUSE:
        args.physical_key = MOUSE(key);
        break;
    }
    args.modifiers = 0;

    // add to triggered keys if keydown event and launch the event
    if (action == GLFW_PRESS) {
        m_triggered_keys.insert(args.physical_key);
        m_tick_handler_triggered_keys.insert(args.physical_key);
        m_input_events["key_down"](args);
    } else {
        m_input_events["key_up"](args);
    }
}

void xd::window::update()
{
    // this is used to keep track which triggered keys list triggered() uses
    m_in_update = true;

    // clear the triggered keys
    m_triggered_keys.clear();

    // trigger input callbacks
    glfwPollEvents();

    update_joysticks();

    // update ticks
    m_last_ticks = m_current_ticks;
    m_current_ticks = static_cast<std::uint32_t>(glfwGetTime() * 1000);

    // invoke tick handler if necessary
    if (m_tick_handler) {
        m_tick_handler_counter += delta_ticks();
        while (m_tick_handler_counter >= m_tick_handler_interval) {
            m_tick_handler();
            m_tick_handler_counter -= m_tick_handler_interval;
            m_tick_handler_triggered_keys.clear();
        }
    }

    // calculate fps
    while (m_current_ticks >= (m_last_fps_update + 1000)) {
        m_fps = m_frame_count;
        m_frame_count = 0;
        m_last_fps_update += 1000;
    }

    // increase frame count
    m_frame_count++;
    m_in_update = false;
}

void xd::window::update_joysticks()
{
    if (!m_joystick_enabled) return;

    for (auto& pair : m_joystick_states) {
        int joystick_id = pair.first;
        auto& joystick_state = pair.second;

        auto& buttons = joystick_state.buttons;
        auto& axes = joystick_state.axes;

        int button_count;
        int axes_count;

        int gamepad_success = GLFW_FALSE;
        if (m_gamepad_detection && glfwJoystickIsGamepad(joystick_id)) {
            GLFWgamepadstate state;
            gamepad_success = glfwGetGamepadState(joystick_id, &state);
            if (gamepad_success == GLFW_TRUE) {
                button_count = GLFW_GAMEPAD_BUTTON_LAST + 1;
                axes_count = GLFW_GAMEPAD_AXIS_LAST + 1;
                for (int button = 0; button < button_count; ++button) {
                    buttons[button] = state.buttons[button];
                }
                for (int axis = 0; axis < axes_count; ++axis) {
                    axes[axis] = state.axes[axis];
                }
            }
        }

        if (gamepad_success == GLFW_FALSE) {
            auto button_state = glfwGetJoystickButtons(0, &button_count);
            auto axis_state = glfwGetJoystickAxes(0, &axes_count);

            for (int button = 0; button < button_count; button++)
            {
                buttons[button] = button_state[button];
            }

            int hat_count;
            auto hat_state = glfwGetJoystickHats(joystick_id, &hat_count);
            if (hat_count > 0) {
               buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] = hat_state[0] & GLFW_HAT_UP;
               buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = hat_state[0] & GLFW_HAT_RIGHT;
               buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] = hat_state[0] & GLFW_HAT_DOWN;
               buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = hat_state[0] & GLFW_HAT_LEFT;
            }

            for (int axis = 0; axis < axes_count; axis++) {
                axes[axis] = axis_state[axis];
            }
        }

        if (m_axis_as_dpad) {
            buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] |=
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_Y] <= -m_axis_sensitivity);
            buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] |=
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_X] >= m_axis_sensitivity);
            buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] |=
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_Y] >= m_axis_sensitivity);
            buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] |=
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_X] <= -m_axis_sensitivity);
        }

        for (int button = 0; button < button_count; button++) {
            if (buttons[button] != joystick_state.prev_buttons[button]) {
                on_input(input_type::INPUT_GAMEPAD, button, buttons[button], joystick_id);
                joystick_state.prev_buttons[button] = buttons[button];
            }
        }
    }
}

void xd::window::clear()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void xd::window::swap()
{
    glfwSwapBuffers(m_window);
}

bool xd::window::focused() const {
    return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) == GL_TRUE;
}

bool xd::window::closed() const
{
    return glfwWindowShouldClose(m_window) != 0;
}

int xd::window::width() const
{
    int width;
    glfwGetWindowSize(m_window, &width, nullptr);
    return width;
}

int xd::window::height() const
{
    int height;
    glfwGetWindowSize(m_window, nullptr, &height);
    return height;
}

int xd::window::framebuffer_width() const
{
    int width;
    glfwGetFramebufferSize(m_window, &width, nullptr);
    return width;
}

int xd::window::framebuffer_height() const
{
    int height;
    glfwGetFramebufferSize(m_window, nullptr, &height);
    return height;
}

void xd::window::set_size(int width, int height)
{
    m_width = width;
    m_height = height;
    glfwSetWindowSize(m_window, width, height);
}

xd::vec2 xd::window::get_size() const
{
    auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    return mode ? xd::vec2{ mode->width, mode->height } : xd::vec2{ 0.0f, 0.0f };
}

std::vector<xd::vec2> xd::window::get_sizes() const
{
    std::vector<xd::vec2> sizes;
    std::unordered_set<xd::vec2> size_map;

    int count;
    const auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
    if (modes) {
        for (int i = 0; i < count; ++i) {
            xd::vec2 size{modes[i].width, modes[i].height};
            if (size_map.find(size) == size_map.end()) {
                sizes.push_back(size);
                size_map.insert(size);
            }
        }
    }

    return sizes;
}

bool xd::window::is_fullscreen() const {
    return glfwGetWindowMonitor(m_window) != nullptr;
}

void xd::window::set_fullscreen(bool fullscreen) {
    if (is_fullscreen() == fullscreen) return;

    if (fullscreen) {
        glfwGetWindowPos(m_window, &m_windowed_pos.x, &m_windowed_pos.y);
        glfwGetWindowSize(m_window, &m_windowed_size.x, &m_windowed_size.y);
        auto res = get_size();
        glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0,
            static_cast<int>(res.x), static_cast<int>(res.y), GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(m_window, nullptr, m_windowed_pos.x, m_windowed_pos.y,
            m_windowed_size.x, m_windowed_size.y, GLFW_DONT_CARE);
    }
}

void xd::window::set_vsync(bool vsync) const {
    glfwSwapInterval(vsync ? 1 : 0);
}

void xd::window::set_gamma(float gamma) const {
    if (gamma < 0.01f) gamma = 0.01f;
    glfwSetGamma(glfwGetPrimaryMonitor(), gamma);
}

void xd::window::bind_key(const xd::key& physical_key, const std::string& virtual_key)
{
    // find if the physical key is bound
    xd::window::key_table_t::iterator i = m_key_to_virtual.find(physical_key);
    if (i == m_key_to_virtual.end()) {
        // if it's not found, add it to the tables
        m_key_to_virtual[physical_key] = virtual_key;
        m_virtual_to_key[virtual_key].insert(physical_key);
    }
}

void xd::window::unbind_key(const xd::key& physical_key)
{
    // find if the physical key is bound
    xd::window::key_table_t::iterator i = m_key_to_virtual.find(physical_key);
    if (i != m_key_to_virtual.end()) {
        // erase physical key from the virtual key's list
        m_virtual_to_key[i->second].erase(physical_key);
        // erase the physical key itself
        m_key_to_virtual.erase(i);
    }
}

void xd::window::unbind_key(const std::string& virtual_key)
{
    // find if the virtual key is bound
    xd::window::virtual_table_t::iterator i = m_virtual_to_key.find(virtual_key);
    if (i != m_virtual_to_key.end()) {
        // erase all the keys associated to this virtual key
        for (xd::window::key_set_t::iterator j = i->second.begin(); j != i->second.end(); ++j) {
            m_key_to_virtual.erase(*j);
        }
        // erase the virtual key itself
        m_virtual_to_key.erase(i);
    }
}

bool xd::window::pressed(const xd::key& key, int joystick_id) const
{
    switch (key.type) {
    case xd::input_type::INPUT_KEYBOARD:
        return glfwGetKey(m_window, key.code) == GLFW_PRESS;
    case xd::input_type::INPUT_MOUSE:
        return  glfwGetMouseButton(m_window, key.code) == GLFW_PRESS;
    case xd::input_type::INPUT_GAMEPAD:
        if (joystick_id == -1 && first_joystick_id() != -1) {
            for (auto& [id, state] : m_joystick_states) {
                if (state.buttons[key.code] == GLFW_PRESS)
                    return true;
            }
        }
        return joystick_present(joystick_id) &&
            m_joystick_states.at(joystick_id).buttons[key.code] == GLFW_PRESS;
    default:
        return false;
    }
}

bool xd::window::pressed(const std::string& key, int joystick_id) const
{
    // find if this virtual key is bound
    xd::window::virtual_table_t::const_iterator i = m_virtual_to_key.find(key);
    if (i != m_virtual_to_key.end()) {
        // iterate through each physical key
        for (xd::window::key_set_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
            if (pressed(*j, joystick_id))
                    return true;
        }
    }
    return false;
}

bool xd::window::triggered(const xd::key& key, int joystick_id) const
{
    auto& key_set = m_in_update ? m_tick_handler_triggered_keys : m_triggered_keys;
    if (key_set.empty()) return false;
    auto key_copy = key;
    if (key_copy.type == input_type::INPUT_GAMEPAD) {
        if (joystick_id == -1 && first_joystick_id() != -1) {
            for (auto& candidate : key_set) {
                if (candidate.type == key.type && candidate.code == key.code)
                    key_copy.device_id = candidate.device_id;
            }
        } else {
            key_copy.device_id = joystick_id;
        }
    }
    return key_set.find(key_copy) != key_set.end();
}

bool xd::window::triggered(const std::string& key, int joystick_id) const
{
    // find if this virtual key is bound
    xd::window::virtual_table_t::const_iterator i = m_virtual_to_key.find(key);
    if (i != m_virtual_to_key.end()) {
        // iterate through each physical key
        for (xd::window::key_set_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
            if (triggered(*j, joystick_id))
                return true;
        }
    }
    return false;
}

bool xd::window::triggered_once(const xd::key& key, int joystick_id)
{
    auto& key_set = m_in_update ? m_tick_handler_triggered_keys : m_triggered_keys;
    if (key_set.empty()) return false;
    auto key_copy = key;
    if (key_copy.type == input_type::INPUT_GAMEPAD) {
        if (joystick_id == -1 && first_joystick_id() != -1) {
            for (auto& candidate : key_set) {
                if (candidate.type == key.type && candidate.code == key.code)
                    key_copy.device_id = candidate.device_id;
            }
        } else {
            key_copy.device_id = joystick_id;
        }
    }
    if (triggered(key, joystick_id)) {
        key_set.erase(key_copy);
        return true;
    }
    return false;
}

bool xd::window::triggered_once(const std::string& key, int joystick_id)
{
    auto i = m_virtual_to_key.find(key);
    if (i != m_virtual_to_key.end()) {
        for (auto j = i->second.begin(); j != i->second.end(); ++j) {
            if (triggered_once(*j, joystick_id))
                return true;
        }
    }
    return false;
}

float xd::window::axis_value(const xd::key& key, int joystick_id)
{
    if (key.type != input_type::INPUT_GAMEPAD) return 0.0f;
    if (joystick_id == -1 && first_joystick_id() != -1) {
        for (auto& [id, state] : m_joystick_states) {
            if (state.axes[key.code] != 0.0f)
                joystick_id = id;
        }
    }
    if (!joystick_present(joystick_id)) return 0.0f;

    return m_joystick_states[joystick_id].axes[key.code];
}

float xd::window::axis_value(const std::string& key, int joystick_id)
{
    // find if this virtual key is bound
    xd::window::virtual_table_t::const_iterator i = m_virtual_to_key.find(key);
    if (i != m_virtual_to_key.end()) {
        // iterate through each physical key and return first matching one
        for (xd::window::key_set_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
            return axis_value(*j, joystick_id);
        }
    }
    return 0.0f;
}

int xd::window::first_joystick_id() const
{
    if (!m_joystick_states.empty()) {
        // We iterate over GLFW_JOYSTICK_X instead of states to make sure we pick smallest ID
        for (int joystick = GLFW_JOYSTICK_1; joystick < GLFW_JOYSTICK_LAST + 1; ++joystick) {
            if (joystick_present(joystick))
                return joystick;
        }
    }
    return -1;
}

std::unordered_map<int, std::string> xd::window::joystick_names() const
{
    std::unordered_map<int, std::string> names;

    for (auto& [id, state] : m_joystick_states) {
        const char* name = nullptr;
        if (joystick_is_gamepad(id)) {
            name = glfwGetGamepadName(id);
        } else {
            name = glfwGetJoystickName(id);
        }
        if (name) {
            names[id] = name;
        }
    }

    return names;
}

void xd::window::reset_joystick_states()
{
    for (int joystick = GLFW_JOYSTICK_1; joystick < GLFW_JOYSTICK_LAST + 1; ++joystick) {
        if (glfwJoystickPresent(joystick)) {
            add_joystick(joystick);
        } else {
            remove_joystick(joystick);
        }
    }
}

