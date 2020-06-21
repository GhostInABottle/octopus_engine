#include "../../../include/xd/system/input.hpp"
#include <GL/glfw3.h>

namespace xd
{
    // pre-defined keyboard keys
    const key KEY_LEFT = KEY(GLFW_KEY_LEFT);
    const key KEY_RIGHT= KEY(GLFW_KEY_RIGHT);
    const key KEY_UP = KEY(GLFW_KEY_UP);
    const key KEY_DOWN = KEY(GLFW_KEY_DOWN);
    const key KEY_ENTER = KEY(GLFW_KEY_ENTER);
    const key KEY_SPACE = KEY(GLFW_KEY_SPACE);
    const key KEY_ESC = KEY(GLFW_KEY_ESCAPE);
    const key KEY_LCTRL = KEY(GLFW_KEY_LEFT_CONTROL);
    const key KEY_RCTRL = KEY(GLFW_KEY_RIGHT_CONTROL);
    const key KEY_LALT = KEY(GLFW_KEY_LEFT_ALT);
    const key KEY_RALT = KEY(GLFW_KEY_RIGHT_ALT);
    const key KEY_LSHIFT = KEY(GLFW_KEY_LEFT_SHIFT);
    const key KEY_RSHIFT = KEY(GLFW_KEY_RIGHT_SHIFT);
    const key KEY_A = KEY(GLFW_KEY_A);
    const key KEY_B = KEY(GLFW_KEY_B);
    const key KEY_C = KEY(GLFW_KEY_C);
    const key KEY_D = KEY(GLFW_KEY_D);
    const key KEY_E = KEY(GLFW_KEY_E);
    const key KEY_F = KEY(GLFW_KEY_F);
    const key KEY_G = KEY(GLFW_KEY_G);
    const key KEY_H = KEY(GLFW_KEY_H);
    const key KEY_I = KEY(GLFW_KEY_I);
    const key KEY_J = KEY(GLFW_KEY_J);
    const key KEY_K = KEY(GLFW_KEY_K);
    const key KEY_L = KEY(GLFW_KEY_L);
    const key KEY_M = KEY(GLFW_KEY_M);
    const key KEY_N = KEY(GLFW_KEY_N);
    const key KEY_O = KEY(GLFW_KEY_O);
    const key KEY_P = KEY(GLFW_KEY_P);
    const key KEY_Q = KEY(GLFW_KEY_Q);
    const key KEY_R = KEY(GLFW_KEY_R);
    const key KEY_S = KEY(GLFW_KEY_S);
    const key KEY_T = KEY(GLFW_KEY_T);
    const key KEY_U = KEY(GLFW_KEY_U);
    const key KEY_V = KEY(GLFW_KEY_V);
    const key KEY_W = KEY(GLFW_KEY_W);
    const key KEY_X = KEY(GLFW_KEY_X);
    const key KEY_Y = KEY(GLFW_KEY_Y);
    const key KEY_Z = KEY(GLFW_KEY_Z);
    const key KEY_0 = KEY(GLFW_KEY_0);
    const key KEY_1 = KEY(GLFW_KEY_1);
    const key KEY_2 = KEY(GLFW_KEY_2);
    const key KEY_3 = KEY(GLFW_KEY_3);
    const key KEY_4 = KEY(GLFW_KEY_4);
    const key KEY_5 = KEY(GLFW_KEY_5);
    const key KEY_6 = KEY(GLFW_KEY_6);
    const key KEY_7 = KEY(GLFW_KEY_7);
    const key KEY_8 = KEY(GLFW_KEY_8);
    const key KEY_9 = KEY(GLFW_KEY_9);

    // pre-defined mouse keys
    const key MOUSE_LEFT = MOUSE(GLFW_MOUSE_BUTTON_LEFT);
    const key MOUSE_RIGHT = MOUSE(GLFW_MOUSE_BUTTON_RIGHT);
    const key MOUSE_MIDDLE = MOUSE(GLFW_MOUSE_BUTTON_MIDDLE);
    const key MOUSE_1 = MOUSE(GLFW_MOUSE_BUTTON_1);
    const key MOUSE_2 = MOUSE(GLFW_MOUSE_BUTTON_2);
    const key MOUSE_3 = MOUSE(GLFW_MOUSE_BUTTON_3);
    const key MOUSE_4 = MOUSE(GLFW_MOUSE_BUTTON_4);
    const key MOUSE_5 = MOUSE(GLFW_MOUSE_BUTTON_5);
    const key MOUSE_6 = MOUSE(GLFW_MOUSE_BUTTON_6);
    const key MOUSE_7 = MOUSE(GLFW_MOUSE_BUTTON_7);
    const key MOUSE_8 = MOUSE(GLFW_MOUSE_BUTTON_8);

    // pre-defined gamepad keys
    const key GAMEPAD_BUTTON_A = GAMEPAD(GLFW_GAMEPAD_BUTTON_A);
    const key GAMEPAD_BUTTON_B = GAMEPAD(GLFW_GAMEPAD_BUTTON_B);
    const key GAMEPAD_BUTTON_X = GAMEPAD(GLFW_GAMEPAD_BUTTON_X);
    const key GAMEPAD_BUTTON_Y = GAMEPAD(GLFW_GAMEPAD_BUTTON_Y);
    const key GAMEPAD_BUTTON_LEFT_BUMPER = GAMEPAD(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);
    const key GAMEPAD_BUTTON_RIGHT_BUMPER = GAMEPAD(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
    const key GAMEPAD_BUTTON_BACK = GAMEPAD(GLFW_GAMEPAD_BUTTON_BACK);
    const key GAMEPAD_BUTTON_START = GAMEPAD(GLFW_GAMEPAD_BUTTON_START);
    const key GAMEPAD_BUTTON_GUIDE = GAMEPAD(GLFW_GAMEPAD_BUTTON_GUIDE);
    const key GAMEPAD_BUTTON_LEFT_THUMB = GAMEPAD(GLFW_GAMEPAD_BUTTON_LEFT_THUMB);
    const key GAMEPAD_BUTTON_RIGHT_THUMB = GAMEPAD(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB);
    const key GAMEPAD_BUTTON_DPAD_UP = GAMEPAD(GLFW_GAMEPAD_BUTTON_DPAD_UP);
    const key GAMEPAD_BUTTON_DPAD_RIGHT = GAMEPAD(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);
    const key GAMEPAD_BUTTON_DPAD_DOWN = GAMEPAD(GLFW_GAMEPAD_BUTTON_DPAD_DOWN);
    const key GAMEPAD_BUTTON_DPAD_LEFT = GAMEPAD(GLFW_GAMEPAD_BUTTON_DPAD_LEFT);

    const key GAMEPAD_AXIS_LEFT_X = GAMEPAD(GLFW_GAMEPAD_AXIS_LEFT_X);
    const key GAMEPAD_AXIS_LEFT_Y =  GAMEPAD(GLFW_GAMEPAD_AXIS_LEFT_Y);
    const key GAMEPAD_AXIS_RIGHT_X =  GAMEPAD(GLFW_GAMEPAD_AXIS_RIGHT_X);
    const key GAMEPAD_AXIS_RIGHT_Y =  GAMEPAD(GLFW_GAMEPAD_AXIS_RIGHT_Y);
    const key GAMEPAD_AXIS_LEFT_TRIGGER =  GAMEPAD(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);
    const key GAMEPAD_AXIS_RIGHT_TRIGGER =  GAMEPAD(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);
}
