#include "../../../include/xd/system/window.hpp"
#include "../../../include/xd/system/exceptions.hpp"
#include "../../../include/xd/vendor/glm/gtx/hash.hpp"
#include "../../../include/xd/vendor/utf8.h"
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

    void on_character_input(GLFWwindow*, unsigned int codepoint) {
        window_instance->on_character_input(codepoint);
    }

    void on_joystick_changed(int id, int event) {
        window_instance->on_joystick_changed(id, event);
    }

    void on_error(int error, const char* description) {
        window_instance->on_error(error, description);
    }
};

xd::window::window(const std::string& title, int width, int height, const window_options& options)
    : m_width(width)
    , m_height(height)
    , m_last_input_type(input_type::INPUT_KEYBOARD)
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
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Calculate windowed size
    auto screen_size = get_size();
    auto missing_size = m_width == -1 || m_height == -1;
    if (!missing_size && !options.fullscreen) {
        m_windowed_size = xd::vec2(m_width, m_height);
    } else {
        auto screen_dim = screen_size.x < screen_size.y ? screen_size.x : screen_size.y;
        auto game_dim = static_cast<float>(screen_size.x < screen_size.y ? options.game_width : options.game_height);
        int factor = 1;
        while (factor * game_dim / screen_dim < options.max_windowed_size_percentage) {
            factor++;
        }
        m_windowed_size = xd::ivec2(options.game_width, options.game_height) * (factor - 1);
    }

    if (missing_size && options.fullscreen) {
        m_width = static_cast<int>(screen_size.x);
        m_height = static_cast<int>(screen_size.y);
    } else if (missing_size) {
        m_width = m_windowed_size.x;
        m_height = m_windowed_size.y;
    }

    m_window = glfwCreateWindow(m_width, m_height, title.c_str(),
            options.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

    if (!m_window)
    {
        glfwTerminate();
        throw xd::window_creation_failed();
    }

    // Calculate windowed position
    m_windowed_pos.x = (static_cast<int>(screen_size.x) - m_windowed_size.x) / 2;
    m_windowed_pos.y = (static_cast<int>(screen_size.y) - m_windowed_size.y) / 2;
    glfwSetWindowPos(m_window, m_windowed_pos.x, m_windowed_pos.y);
    glfwShowWindow(m_window);

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

    // initialize ticks
    m_current_ticks = m_last_ticks = m_last_fps_update = static_cast<std::uint32_t>(glfwGetTime() * 1000);
    m_fps = m_frame_count = 0;

    // register input callbacks
    glfwSetKeyCallback(m_window, &on_key_proxy);
    glfwSetMouseButtonCallback(m_window, &on_mouse_proxy);
    glfwSetJoystickCallback(&::on_joystick_changed);

    glfwSetErrorCallback(&::on_error);

    window_instance = this;

    m_joystick_enabled = options.enable_joystick;
    m_gamepad_detection = options.gamepad_detection;
    m_axis_as_dpad = options.axis_as_dpad;
    m_stick_sensitivity = options.stick_sensitivity;
    m_trigger_sensitivity = options.trigger_sensitivity;

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
        // + 3 because we add two pseudo-buttons for the triggers
        for (int button = 0; button < GLFW_GAMEPAD_BUTTON_LAST + 3; ++button) {
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

    m_last_input_type = type;
}

void xd::window::on_character_input(unsigned int codepoint) {
    utf8::append(codepoint, std::back_inserter(m_character_buffer));
}

void xd::window::on_joystick_changed(int id, int event) {
    if (event == GLFW_CONNECTED) {
        m_joysticks_to_add.push_back(id);
    } else if (event == GLFW_DISCONNECTED) {
        m_joysticks_to_remove.push_back(id);
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

    if (!m_joysticks_to_add.empty()) {
        for (int id : m_joysticks_to_add) {
            add_joystick(id);
        }
        m_joysticks_to_add.clear();
    }

    if (!m_joysticks_to_remove.empty()) {
        for (int id : m_joysticks_to_remove) {
            remove_joystick(id);
        }
        m_joysticks_to_remove.clear();
    }

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

            if (button_count > GLFW_GAMEPAD_BUTTON_LAST + 1) {
                button_count = GLFW_GAMEPAD_BUTTON_LAST + 1;
            }

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
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_Y] <= -m_stick_sensitivity);
            buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] |=
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_X] >= m_stick_sensitivity);
            buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] |=
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_Y] >= m_stick_sensitivity);
            buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] |=
                static_cast<unsigned char>(axes[GLFW_GAMEPAD_AXIS_LEFT_X] <= -m_stick_sensitivity);
        }

        for (int button = 0; button < button_count; button++) {
            if (buttons[button] != joystick_state.prev_buttons[button]) {
                on_input(input_type::INPUT_GAMEPAD, button, buttons[button], joystick_id);
                joystick_state.prev_buttons[button] = buttons[button];
            }
        }

        // Triggers as buttons
        process_trigger(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, joystick_state, joystick_id);
        process_trigger(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, joystick_state, joystick_id);
    }
}

void xd::window::process_trigger(int axis, xd::window::joystick_state& state, int joystick_id) {
    int index = axis == GLFW_GAMEPAD_AXIS_LEFT_TRIGGER
        ? GAMEPAD_BUTTON_LEFT_TRIGGER_INDEX
        : GAMEPAD_BUTTON_RIGHT_TRIGGER_INDEX;
    state.buttons[index] = static_cast<unsigned char>(state.axes[axis] >= m_trigger_sensitivity);
    if (state.buttons[index] != state.prev_buttons[index]) {
        on_input(input_type::INPUT_GAMEPAD, index, state.buttons[index], joystick_id);
        state.prev_buttons[index] = state.buttons[index];
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
    auto fullscreen = is_fullscreen();
    auto missing_size = width == -1 || height == -1;
    if (missing_size && fullscreen) {
        auto size = get_size();
        width = static_cast<int>(size.x);
        height = static_cast<int>(size.y);
    } else if (missing_size) {
        width = m_windowed_size.x;
        height = m_windowed_size.y;
    }

    m_width = width;
    m_height = height;
    glfwSetWindowSize(m_window, width, height);
}

xd::vec2 xd::window::get_size() const
{
    auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    return mode ? xd::vec2{mode->width, mode->height} : xd::vec2{0.0f, 0.0f};
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
    auto monitor = glfwGetWindowMonitor(m_window);
    return monitor != nullptr;
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

void xd::window::set_icons(std::vector<std::shared_ptr<xd::image>> icon_images) const {
    if (icon_images.empty()) return;

    std::vector<GLFWimage> glfw_images;
    for (auto& icon : icon_images) {
        GLFWimage image{};
        image.width = icon->width();
        image.height = icon->height();
        image.pixels = static_cast<unsigned char*>(icon->data());
        glfw_images.push_back(image);
    }

    glfwSetWindowIcon(m_window, glfw_images.size(), glfw_images.data());
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


std::string xd::window::key_name(const key& physical_key)
{
    const char* name = glfwGetKeyName(physical_key.code, 0);
    return name ? name : "";
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
            if (pressed(*j, joystick_id)) return true;
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

void xd::window::begin_character_input() const {
    glfwSetCharCallback(m_window, ::on_character_input);
}

std::string xd::window::end_character_input() {
    glfwSetCharCallback(m_window, nullptr);
    std::string input_copy{m_character_buffer};
    m_character_buffer = "";
    return input_copy;
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

std::optional<std::string> xd::window::first_joystick_name() const
{
    auto id = first_joystick_id();
    const char* name = nullptr;
    if (joystick_is_gamepad(id)) {
        name = glfwGetGamepadName(id);
    } else {
        name = glfwGetJoystickName(id);
    }
    return name ? std::optional<std::string>{name} : std::nullopt;
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

