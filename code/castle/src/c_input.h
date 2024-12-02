#pragma once

#include <GLFW/glfw3.h>
#include <castle_common/cc_math.h>

using KeysDownBits = unsigned long long;
using MouseButtonsDownBits = unsigned char;
using GamepadButtonsDownBits = unsigned short;

enum KeyCode
{
    KEY_SPACE,

    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,

    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_ALT,

    KEY_CODE_CNT
};

enum MouseButtonCode
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MID,

    MOUSE_BUTTON_CODE_CNT
};

enum GamepadButtonCode
{
    GAMEPAD_BUTTON_A,
    GAMEPAD_BUTTON_B,
    GAMEPAD_BUTTON_X,
    GAMEPAD_BUTTON_Y,

    GAMEPAD_BUTTON_LEFT_BUMPER,
    GAMEPAD_BUTTON_RIGHT_BUMPER,

    GAMEPAD_BUTTON_BACK,
    GAMEPAD_BUTTON_START,
    GAMEPAD_BUTTON_GUIDE,

    GAMEPAD_BUTTON_LEFT_THUMB,
    GAMEPAD_BUTTON_RIGHT_THUMB,

    GAMEPAD_BUTTON_DPAD_UP,
    GAMEPAD_BUTTON_DPAD_RIGHT,
    GAMEPAD_BUTTON_DPAD_DOWN,
    GAMEPAD_BUTTON_DPAD_LEFT,

    GAMEPAD_BUTTON_CODE_CNT
};

enum GamepadAxisCode
{
    GAMEPAD_AXIS_LEFT_X,
    GAMEPAD_AXIS_LEFT_Y,

    GAMEPAD_AXIS_RIGHT_X,
    GAMEPAD_AXIS_RIGHT_Y,

    GAMEPAD_AXIS_LEFT_TRIGGER,
    GAMEPAD_AXIS_RIGHT_TRIGGER,

    GAMEPAD_AXIS_CODE_CNT
};

struct GamepadState
{
    bool connected; // Included to support zero-initialisation.
    int glfwJoystickIndex;
    GamepadButtonsDownBits buttonsDownBits;
    float axisValues[GAMEPAD_AXIS_CODE_CNT];
};

struct InputState
{
    KeysDownBits keysDownBits;

    MouseButtonsDownBits mouseButtonsDownBits;
    cc::Vec2D mousePos;
    int mouseScroll;

    GamepadState gamepadState;
};

class InputManager
{
public:
    void refresh_states(GLFWwindow *const glfwWindow, const int mouseScroll);

    inline bool is_key_down(const KeyCode keyCode) const
    {
        const auto keyBit = static_cast<KeysDownBits>(1) << keyCode;
        return m_state.keysDownBits & keyBit;
    }

    inline bool is_key_pressed(const KeyCode keyCode) const
    {
        const auto keyBit = static_cast<KeysDownBits>(1) << keyCode;
        return (m_state.keysDownBits & keyBit) && !(m_stateLast.keysDownBits & keyBit);
    }

    inline bool is_key_released(const KeyCode keyCode) const
    {
        const auto keyBit = static_cast<KeysDownBits>(1) << keyCode;
        return !(m_state.keysDownBits & keyBit) && (m_stateLast.keysDownBits & keyBit);
    }

    inline bool is_mouse_button_down(const MouseButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<MouseButtonsDownBits>(1) << buttonCode;
        return m_state.mouseButtonsDownBits & buttonBit;
    }

    inline bool is_mouse_button_pressed(const MouseButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<MouseButtonsDownBits>(1) << buttonCode;
        return (m_state.mouseButtonsDownBits & buttonBit) && !(m_stateLast.mouseButtonsDownBits & buttonBit);
    }

    inline bool is_mouse_button_released(const MouseButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<MouseButtonsDownBits>(1) << buttonCode;
        return !(m_state.mouseButtonsDownBits & buttonBit) && (m_stateLast.mouseButtonsDownBits & buttonBit);
    }

    inline cc::Vec2D get_mouse_pos() const
    {
        return m_state.mousePos;
    }

    inline int get_mouse_scroll() const
    {
        return m_state.mouseScroll;
    }

    inline bool is_gamepad_connected() const
    {
        return m_state.gamepadState.connected;
    }

    inline bool is_gamepad_button_down(const GamepadButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<GamepadButtonsDownBits>(1) << buttonCode;
        return m_state.gamepadState.buttonsDownBits & buttonBit;
    }

    inline bool is_gamepad_button_pressed(const GamepadButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<GamepadButtonsDownBits>(1) << buttonCode;
        return (m_state.gamepadState.buttonsDownBits & buttonBit) && !(m_stateLast.gamepadState.buttonsDownBits & buttonBit);
    }

    inline bool is_gamepad_button_released(const GamepadButtonCode buttonCode) const
    {
        const auto buttonBit = static_cast<GamepadButtonsDownBits>(1) << buttonCode;
        return !(m_state.gamepadState.buttonsDownBits & buttonBit) && (m_stateLast.gamepadState.buttonsDownBits & buttonBit);
    }

    inline float get_gamepad_axis_value(const GamepadAxisCode axisCode) const
    {
        return m_state.gamepadState.axisValues[axisCode];
    }

private:
    InputState m_state;
    InputState m_stateLast;
};

InputState make_input_state(GLFWwindow *const glfwWindow, const int mouseScroll);
