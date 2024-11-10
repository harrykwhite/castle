#pragma once

#include <GLFW/glfw3.h>
#include <castle_common/cc_math.h>

enum class ec_key_code
{
    space,

    num_0,
    num_1,
    num_2,
    num_3,
    num_4,
    num_5,
    num_6,
    num_7,
    num_8,
    num_9,

    a,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    k,
    l,
    m,
    n,
    o,
    p,
    q,
    r,
    s,
    t,
    u,
    v,
    w,
    x,
    y,
    z,

    escape,
    enter,
    tab,

    right,
    left,
    down,
    up,

    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
    f10,
    f11,
    f12,

    left_shift,
    left_control,
    left_alt
};

enum class ec_mouse_button_code
{
    left,
    right,
    middle
};

enum class ec_gamepad_button_code
{
    a,
    b,
    x,
    y,

    left_bumper,
    right_bumper,

    back,
    start,
    guide,

    left_thumb,
    right_thumb,

    dpad_up,
    dpad_right,
    dpad_down,
    dpad_left
};

enum class ec_gamepad_axis_code
{
    left_x,
    left_y,

    right_x,
    right_y,

    left_trigger,
    right_trigger
};

using u_keys_down_bits = unsigned long long;
using u_mouse_buttons_down_bits = unsigned char;
using u_gamepad_buttons_down_bits = unsigned short;

constexpr int k_key_code_cnt = (int)ec_key_code::left_alt;
constexpr int k_mouse_button_code_cnt = (int)ec_mouse_button_code::middle;
constexpr int k_gamepad_button_code_cnt = (int)ec_gamepad_button_code::dpad_left;
constexpr int k_gamepad_axis_code_cnt = (int)ec_gamepad_axis_code::right_trigger;

class c_input_state
{
public:
    void refresh(GLFWwindow *const glfw_window);

    u_keys_down_bits get_keys_down_bits() const
    {
        return m_keys_down_bits;
    }

    u_mouse_buttons_down_bits get_mouse_buttons_down_bits() const
    {
        return m_mouse_buttons_down_bits;
    }

    u_gamepad_buttons_down_bits get_gamepad_buttons_down_bits() const
    {
        return m_gamepad_buttons_down_bits;
    }

private:
    u_keys_down_bits m_keys_down_bits = 0;

    cc::s_vec_2d m_mouse_pos = {};
    u_mouse_buttons_down_bits m_mouse_buttons_down_bits = 0;
    int m_mouse_scroll = 0;

    int m_gamepad_glfw_joystick_index = -1;
    u_gamepad_buttons_down_bits m_gamepad_buttons_down_bits = 0;
    float m_gamepad_axis_values[k_gamepad_axis_code_cnt] = {};
};

class c_input_manager
{
public:
    void refresh(GLFWwindow *const glfw_window);

    inline bool is_key_down(const ec_key_code key_code) const
    {
        return (m_input_state.get_keys_down_bits() & (static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code))) != 0;
    }

    inline bool is_key_pressed(const ec_key_code key_code) const
    {
        return is_key_down(key_code) && !((m_input_state_last.get_keys_down_bits() & (static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code))) != 0);
    }

    inline bool is_key_released(const ec_key_code key_code) const
    {
        return !is_key_down(key_code) && ((m_input_state_last.get_keys_down_bits() & (static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code))) != 0);
    }

    inline bool is_mouse_button_down(const ec_mouse_button_code button_code) const
    {
        return (m_input_state.get_mouse_buttons_down_bits() & (static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code))) != 0;
    }

    inline bool is_mouse_button_pressed(const ec_mouse_button_code button_code) const
    {
        return is_mouse_button_down(button_code) && !((m_input_state_last.get_mouse_buttons_down_bits() & (static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code))) != 0);
    }

    inline bool is_mouse_button_released(const ec_mouse_button_code button_code) const
    {
        return !is_mouse_button_down(button_code) && ((m_input_state_last.get_mouse_buttons_down_bits() & (static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code))) != 0);
    }

    inline bool is_gamepad_button_down(const ec_gamepad_button_code button_code) const
    {
        return (m_input_state.get_gamepad_buttons_down_bits() & (static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code))) != 0;
    }

    inline bool is_gamepad_button_pressed(const ec_gamepad_button_code button_code) const
    {
        return is_gamepad_button_down(button_code) && !((m_input_state_last.get_gamepad_buttons_down_bits() & (static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code))) != 0);
    }

    inline bool is_gamepad_button_released(const ec_gamepad_button_code button_code) const
    {
        return !is_gamepad_button_down(button_code) && ((m_input_state_last.get_gamepad_buttons_down_bits() & (static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code))) != 0);
    }

private:
    c_input_state m_input_state;
    c_input_state m_input_state_last;
};
