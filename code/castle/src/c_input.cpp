#include <castle/c_input.h>

#include <iostream>
#include <castle_common/cc_debugging.h>

static int get_glfw_key_code(const ec_key_code key_code)
{
    switch (key_code)
    {
        case ec_key_code::space: return GLFW_KEY_SPACE;

        case ec_key_code::num_0: return GLFW_KEY_0;
        case ec_key_code::num_1: return GLFW_KEY_1;
        case ec_key_code::num_2: return GLFW_KEY_2;
        case ec_key_code::num_3: return GLFW_KEY_3;
        case ec_key_code::num_4: return GLFW_KEY_4;
        case ec_key_code::num_5: return GLFW_KEY_5;
        case ec_key_code::num_6: return GLFW_KEY_6;
        case ec_key_code::num_7: return GLFW_KEY_7;
        case ec_key_code::num_8: return GLFW_KEY_8;
        case ec_key_code::num_9: return GLFW_KEY_9;

        case ec_key_code::a: return GLFW_KEY_A;
        case ec_key_code::b: return GLFW_KEY_B;
        case ec_key_code::c: return GLFW_KEY_C;
        case ec_key_code::d: return GLFW_KEY_D;
        case ec_key_code::e: return GLFW_KEY_E;
        case ec_key_code::f: return GLFW_KEY_F;
        case ec_key_code::g: return GLFW_KEY_G;
        case ec_key_code::h: return GLFW_KEY_H;
        case ec_key_code::i: return GLFW_KEY_I;
        case ec_key_code::j: return GLFW_KEY_J;
        case ec_key_code::k: return GLFW_KEY_K;
        case ec_key_code::l: return GLFW_KEY_L;
        case ec_key_code::m: return GLFW_KEY_M;
        case ec_key_code::n: return GLFW_KEY_N;
        case ec_key_code::o: return GLFW_KEY_O;
        case ec_key_code::p: return GLFW_KEY_P;
        case ec_key_code::q: return GLFW_KEY_Q;
        case ec_key_code::r: return GLFW_KEY_R;
        case ec_key_code::s: return GLFW_KEY_S;
        case ec_key_code::t: return GLFW_KEY_T;
        case ec_key_code::u: return GLFW_KEY_U;
        case ec_key_code::v: return GLFW_KEY_V;
        case ec_key_code::w: return GLFW_KEY_W;
        case ec_key_code::x: return GLFW_KEY_X;
        case ec_key_code::y: return GLFW_KEY_Y;
        case ec_key_code::z: return GLFW_KEY_Z;

        case ec_key_code::escape: return GLFW_KEY_ESCAPE;
        case ec_key_code::enter: return GLFW_KEY_ENTER;
        case ec_key_code::tab: return GLFW_KEY_TAB;

        case ec_key_code::right: return GLFW_KEY_RIGHT;
        case ec_key_code::left: return GLFW_KEY_LEFT;
        case ec_key_code::down: return GLFW_KEY_DOWN;
        case ec_key_code::up: return GLFW_KEY_UP;

        case ec_key_code::f1: return GLFW_KEY_F1;
        case ec_key_code::f2: return GLFW_KEY_F2;
        case ec_key_code::f3: return GLFW_KEY_F3;
        case ec_key_code::f4: return GLFW_KEY_F4;
        case ec_key_code::f5: return GLFW_KEY_F5;
        case ec_key_code::f6: return GLFW_KEY_F6;
        case ec_key_code::f7: return GLFW_KEY_F7;
        case ec_key_code::f8: return GLFW_KEY_F8;
        case ec_key_code::f9: return GLFW_KEY_F9;
        case ec_key_code::f10: return GLFW_KEY_F10;
        case ec_key_code::f11: return GLFW_KEY_F11;
        case ec_key_code::f12: return GLFW_KEY_F12;

        case ec_key_code::left_shift: return GLFW_KEY_LEFT_SHIFT;
        case ec_key_code::left_control: return GLFW_KEY_LEFT_CONTROL;
        case ec_key_code::left_alt: return GLFW_KEY_LEFT_ALT;
    }

    return GLFW_KEY_UNKNOWN;
}

void c_input_state::refresh(GLFWwindow *const glfw_window)
{
    //
    // Keyboard
    //
    m_keys_down_bits = 0;

    for (int i = 0; i < k_key_code_cnt; ++i)
    {
        if (glfwGetKey(glfw_window, get_glfw_key_code(static_cast<ec_key_code>(i))) == GLFW_PRESS)
        {
            m_keys_down_bits |= static_cast<u_keys_down_bits>(1) << i;
        }
    }

    //
    // Mouse
    //
    m_mouse_buttons_down_bits = 0;

    for (int i = 0; i < k_mouse_button_code_cnt; ++i)
    {
        if (glfwGetMouseButton(glfw_window, i) == GLFW_PRESS)
        {
            m_mouse_buttons_down_bits |= static_cast<u_mouse_buttons_down_bits>(1) << i;
        }
    }

    //
    // Gamepad
    //
    m_gamepad_glfw_joystick_index = -1;
    m_gamepad_buttons_down_bits = 0;
    std::fill(std::begin(m_gamepad_axis_values), std::end(m_gamepad_axis_values), 0.0f);

    // Search for the first active gamepad and if found update the gamepad state using it.
    for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
    {
        if (!glfwJoystickPresent(i) || !glfwJoystickIsGamepad(i))
        {
            continue;
        }

        GLFWgamepadstate glfw_gamepad_state;

        if (!glfwGetGamepadState(i, &glfw_gamepad_state))
        {
            std::cout << "ERROR: Failed to retrieve the state of gamepad with GLFW joystick index " << i << std::endl;
            break;
        }

        m_gamepad_glfw_joystick_index = i;

        // Store which gamepad buttons are down.
        for (int j = 0; j < k_gamepad_button_code_cnt; j++)
        {
            if (glfw_gamepad_state.buttons[j] == GLFW_PRESS)
            {
                m_gamepad_buttons_down_bits |= static_cast<u_gamepad_buttons_down_bits>(1) << j;
            }
        }

        // Store gamepad axis values.
        for (int j = 0; j < k_gamepad_axis_code_cnt; j++)
        {
            m_gamepad_axis_values[j] = glfw_gamepad_state.axes[j];
        }

        break;
    }
}

void c_input_manager::refresh(GLFWwindow *const glfw_window)
{
    m_input_state_last = m_input_state;
    m_input_state.refresh(glfw_window);
}
