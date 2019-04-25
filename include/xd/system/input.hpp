#ifndef H_XD_SYSTEM_INPUT
#define H_XD_SYSTEM_INPUT

#include "../types.hpp"
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <string>

namespace xd
{
    enum input_type
    {
        INPUT_KEYBOARD,
        INPUT_MOUSE,
        INPUT_GAMEPAD
    };

    struct key
    {
        input_type type;
        int code;
        int device_id;
    };

    inline bool operator==(const key& lhs, const key& rhs)
    {
        return (lhs.type == rhs.type
            && lhs.code == rhs.code
            && lhs.device_id == rhs.device_id);
    }

    inline bool operator!=(const key& lhs, const key& rhs)
    {
        return !(lhs == rhs);
    }

    inline std::size_t hash_value(const key& k)
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, k.type);
        boost::hash_combine(seed, k.code);
        boost::hash_combine(seed, k.device_id);
        return seed;
    }

    struct input_args
    {
        key physical_key;
        boost::optional<std::string> virtual_key;
        int modifiers;
    };

    struct input_filter
    {
        input_filter() {}
        input_filter(input_type type, boost::optional<int> mod = boost::none) : type(type), modifiers(mod) {}
        input_filter(const key& pkey, boost::optional<int> mod = boost::none) : physical_key(pkey), modifiers(mod) {}
        input_filter(std::string vkey, boost::optional<int> mod = boost::none) : virtual_key(vkey), modifiers(mod) {}

        boost::optional<input_type> type;
        boost::optional<key> physical_key;
        boost::optional<std::string> virtual_key;
        boost::optional<int> modifiers;

        bool operator()(const input_args& args)
        {
            if (type != boost::none && *type != args.physical_key.type)
                return false;
            if (physical_key != boost::none && *physical_key != args.physical_key)
                return false;
            if (virtual_key != boost::none && *virtual_key != args.virtual_key)
                return false;
            if (modifiers != boost::none && (*modifiers & args.modifiers) != *modifiers)
                return false;
            return true;
        }
    };

    // utility function to create key
    inline key KEY(int code)
    {
        key k;
        k.type = INPUT_KEYBOARD;
        k.code = code;
        k.device_id = -1;
        return k;
    }

    // utility function to create mouse
    inline key MOUSE(int code)
    {
        key k;
        k.type = INPUT_MOUSE;
        k.code = code;
        k.device_id = -1;
        return k;
    }

    // utility function to create gamepad
    inline key GAMEPAD(int code, int joystick_id = -1)
    {
        key k;
        k.type = INPUT_GAMEPAD;
        k.code = code;
        k.device_id = joystick_id;
        return k;
    }

    // pre-defined keyboard keys
    extern const key KEY_LEFT;
    extern const key KEY_RIGHT;
    extern const key KEY_UP;
    extern const key KEY_DOWN;
    extern const key KEY_ENTER;
    extern const key KEY_SPACE;
    extern const key KEY_ESC;
    extern const key KEY_A;
    extern const key KEY_B;
    extern const key KEY_C;
    extern const key KEY_D;
    extern const key KEY_E;
    extern const key KEY_F;
    extern const key KEY_G;
    extern const key KEY_H;
    extern const key KEY_I;
    extern const key KEY_J;
    extern const key KEY_K;
    extern const key KEY_L;
    extern const key KEY_M;
    extern const key KEY_N;
    extern const key KEY_O;
    extern const key KEY_P;
    extern const key KEY_Q;
    extern const key KEY_R;
    extern const key KEY_S;
    extern const key KEY_T;
    extern const key KEY_U;
    extern const key KEY_V;
    extern const key KEY_W;
    extern const key KEY_X;
    extern const key KEY_Y;
    extern const key KEY_Z;
    extern const key KEY_0;
    extern const key KEY_1;
    extern const key KEY_2;
    extern const key KEY_3;
    extern const key KEY_4;
    extern const key KEY_5;
    extern const key KEY_6;
    extern const key KEY_7;
    extern const key KEY_8;
    extern const key KEY_9;

    // pre-defined mouse keys
    extern const key MOUSE_LEFT;
    extern const key MOUSE_RIGHT;
    extern const key MOUSE_MIDDLE;
    extern const key MOUSE_1;
    extern const key MOUSE_2;
    extern const key MOUSE_3;
    extern const key MOUSE_4;
    extern const key MOUSE_5;
    extern const key MOUSE_6;
    extern const key MOUSE_7;
    extern const key MOUSE_8;

    // predefined gamepad/joystick keys
    extern const key GAMEPAD_BUTTON_A;
    extern const key GAMEPAD_BUTTON_B;
    extern const key GAMEPAD_BUTTON_X;
    extern const key GAMEPAD_BUTTON_Y;
    extern const key GAMEPAD_BUTTON_LEFT_BUMPER;
    extern const key GAMEPAD_BUTTON_RIGHT_BUMPER;
    extern const key GAMEPAD_BUTTON_BACK;
    extern const key GAMEPAD_BUTTON_START;
    extern const key GAMEPAD_BUTTON_GUIDE;
    extern const key GAMEPAD_BUTTON_LEFT_THUMB;
    extern const key GAMEPAD_BUTTON_RIGHT_THUMB;
    extern const key GAMEPAD_BUTTON_DPAD_UP;
    extern const key GAMEPAD_BUTTON_DPAD_RIGHT;
    extern const key GAMEPAD_BUTTON_DPAD_DOWN;
    extern const key GAMEPAD_BUTTON_DPAD_LEFT;
    extern const key GAMEPAD_AXIS_LEFT_X;
    extern const key GAMEPAD_AXIS_LEFT_Y;
    extern const key GAMEPAD_AXIS_RIGHT_X;
    extern const key GAMEPAD_AXIS_RIGHT_Y;
    extern const key GAMEPAD_AXIS_LEFT_TRIGGER;
    extern const key GAMEPAD_AXIS_RIGHT_TRIGGER;
}

// specialize hash<> for xd::key
namespace std
{
    template <>
    struct hash<xd::key>
    {
        size_t operator()(const xd::key& k) const
        {
            return xd::hash_value(k);
        }
    };
}

#endif
