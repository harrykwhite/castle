#include "c_input.h"

static int get_glfw_key_code(const KeyCode keyCode)
{
    switch (keyCode)
    {
        case KEY_SPACE: return GLFW_KEY_SPACE;

        case KEY_0: return GLFW_KEY_0;
        case KEY_1: return GLFW_KEY_1;
        case KEY_2: return GLFW_KEY_2;
        case KEY_3: return GLFW_KEY_3;
        case KEY_4: return GLFW_KEY_4;
        case KEY_5: return GLFW_KEY_5;
        case KEY_6: return GLFW_KEY_6;
        case KEY_7: return GLFW_KEY_7;
        case KEY_8: return GLFW_KEY_8;
        case KEY_9: return GLFW_KEY_9;

        case KEY_A: return GLFW_KEY_A;
        case KEY_B: return GLFW_KEY_B;
        case KEY_C: return GLFW_KEY_C;
        case KEY_D: return GLFW_KEY_D;
        case KEY_E: return GLFW_KEY_E;
        case KEY_F: return GLFW_KEY_F;
        case KEY_G: return GLFW_KEY_G;
        case KEY_H: return GLFW_KEY_H;
        case KEY_I: return GLFW_KEY_I;
        case KEY_J: return GLFW_KEY_J;
        case KEY_K: return GLFW_KEY_K;
        case KEY_L: return GLFW_KEY_L;
        case KEY_M: return GLFW_KEY_M;
        case KEY_N: return GLFW_KEY_N;
        case KEY_O: return GLFW_KEY_O;
        case KEY_P: return GLFW_KEY_P;
        case KEY_Q: return GLFW_KEY_Q;
        case KEY_R: return GLFW_KEY_R;
        case KEY_S: return GLFW_KEY_S;
        case KEY_T: return GLFW_KEY_T;
        case KEY_U: return GLFW_KEY_U;
        case KEY_V: return GLFW_KEY_V;
        case KEY_W: return GLFW_KEY_W;
        case KEY_X: return GLFW_KEY_X;
        case KEY_Y: return GLFW_KEY_Y;
        case KEY_Z: return GLFW_KEY_Z;

        case KEY_ESCAPE: return GLFW_KEY_ESCAPE;
        case KEY_ENTER: return GLFW_KEY_ENTER;
        case KEY_TAB: return GLFW_KEY_TAB;

        case KEY_RIGHT: return GLFW_KEY_RIGHT;
        case KEY_LEFT: return GLFW_KEY_LEFT;
        case KEY_DOWN: return GLFW_KEY_DOWN;
        case KEY_UP: return GLFW_KEY_UP;

        case KEY_F1: return GLFW_KEY_F1;
        case KEY_F2: return GLFW_KEY_F2;
        case KEY_F3: return GLFW_KEY_F3;
        case KEY_F4: return GLFW_KEY_F4;
        case KEY_F5: return GLFW_KEY_F5;
        case KEY_F6: return GLFW_KEY_F6;
        case KEY_F7: return GLFW_KEY_F7;
        case KEY_F8: return GLFW_KEY_F8;
        case KEY_F9: return GLFW_KEY_F9;
        case KEY_F10: return GLFW_KEY_F10;
        case KEY_F11: return GLFW_KEY_F11;
        case KEY_F12: return GLFW_KEY_F12;

        case KEY_LEFT_SHIFT: return GLFW_KEY_LEFT_SHIFT;
        case KEY_LEFT_CONTROL: return GLFW_KEY_LEFT_CONTROL;
        case KEY_LEFT_ALT: return GLFW_KEY_LEFT_ALT;

        default: return GLFW_KEY_UNKNOWN;
    }
}

static KeysDownBits get_keys_down_bits(GLFWwindow *const glfwWindow)
{
    KeysDownBits keysDownBits = 0;

    for (int i = 0; i < KEY_CODE_CNT; ++i)
    {
        if (glfwGetKey(glfwWindow, get_glfw_key_code(static_cast<KeyCode>(i))) == GLFW_PRESS)
        {
            keysDownBits |= static_cast<KeysDownBits>(1) << i;
        }
    }

    return keysDownBits;
}

static MouseButtonsDownBits get_mouse_buttons_down_bits(GLFWwindow *const glfwWindow)
{
    MouseButtonsDownBits mouseButtonsDownBits = 0;

    for (int i = 0; i < MOUSE_BUTTON_CODE_CNT; ++i)
    {
        if (glfwGetMouseButton(glfwWindow, i) == GLFW_PRESS)
        {
            mouseButtonsDownBits |= static_cast<MouseButtonsDownBits>(1) << i;
        }
    }

    return mouseButtonsDownBits;
}

static cc::Vec2D get_mouse_pos(GLFWwindow *const glfwWindow)
{
    double mouseXDbl, mouseYDbl;
    glfwGetCursorPos(glfwWindow, &mouseXDbl, &mouseYDbl);
    return {static_cast<float>(mouseXDbl), static_cast<float>(mouseYDbl)};
}

static GamepadState create_gamepad_state()
{
    GamepadState state = {};

    // Search for the first active gamepad and if found update the gamepad state using it.
    for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
    {
        if (!glfwJoystickPresent(i) || !glfwJoystickIsGamepad(i))
        {
            continue;
        }

        GLFWgamepadstate glfwGamepadState;

        if (!glfwGetGamepadState(i, &glfwGamepadState))
        {
            break;
        }

        state.connected = true;
        state.glfwJoystickIndex = i;

        // Store which gamepad buttons are down.
        for (int j = 0; j < GAMEPAD_BUTTON_CODE_CNT; j++)
        {
            if (glfwGamepadState.buttons[j] == GLFW_PRESS)
            {
                state.buttonsDownBits |= static_cast<GamepadButtonsDownBits>(1) << j;
            }
        }

        // Store gamepad axis values.
        for (int j = 0; j < GAMEPAD_AXIS_CODE_CNT; j++)
        {
            state.axisValues[j] = glfwGamepadState.axes[j];
        }

        break;
    }

    return state;
}

InputState InputState::make(GLFWwindow *const glfwWindow, const int mouseScroll)
{
    InputState state;
    state.keysDownBits = get_keys_down_bits(glfwWindow);
    state.mouseButtonsDownBits = get_mouse_buttons_down_bits(glfwWindow);
    state.mousePos = get_mouse_pos(glfwWindow);
    state.mouseScroll = mouseScroll;
    state.gamepadState = create_gamepad_state();
    return state;
}

void InputManager::refresh_states(GLFWwindow *const glfwWindow, const int mouseScroll)
{
    m_stateLast = m_state;
    m_state = InputState::make(glfwWindow, mouseScroll);
}
