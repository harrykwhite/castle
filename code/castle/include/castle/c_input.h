#pragma once

#include <array>
#include <GLFW/glfw3.h>
#include <castle_common/cc_math.h>

enum class ec_key_code
{
    space,
    num_0, num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9,
    a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
    escape, enter, tab,
    right, left, down, up,
    f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
    left_shift, left_control, left_alt,

    POST
};

constexpr int k_key_code_cnt = (int)ec_key_code::POST;

enum class ec_mouse_button_code
{
    left,
    right,
    middle,

    POST
};

constexpr int k_mouse_button_code_cnt = (int)ec_mouse_button_code::POST;

enum class ec_gamepad_button_code
{
    a, b, x, y,
    left_bumper, right_bumper,
    back, start, guide,
    left_thumb, right_thumb,
    dpad_up, dpad_right, dpad_down, dpad_left,

    POST
};

constexpr int k_gamepad_button_code_cnt = (int)ec_gamepad_button_code::POST;

enum class ec_gamepad_axis_code
{
    left_x, left_y,
    right_x, right_y,
    left_trigger, right_trigger,

    POST
};

constexpr int k_gamepad_axis_code_cnt = (int)ec_gamepad_axis_code::POST;

using u_keys_down_bits = unsigned long long;
using u_mouse_buttons_down_bits = unsigned char;
using u_gamepad_buttons_down_bits = unsigned short;

struct s_input_state
{
    u_keys_down_bits keys_down_bits;

    cc::s_vec_2d mouse_pos;
    u_mouse_buttons_down_bits mouse_buttons_down_bits;

    int gamepad_glfw_joystick_index;
    u_gamepad_buttons_down_bits gamepad_buttons_down_bits;
    std::array<float, k_gamepad_axis_code_cnt> gamepad_axis_values;

    s_input_state() = default;

    s_input_state(const u_keys_down_bits keys_down_bits, const cc::s_vec_2d mouse_pos, const u_mouse_buttons_down_bits mouse_buttons_down_bits, const int gamepad_glfw_joystick_index, const u_gamepad_buttons_down_bits gamepad_buttons_down_bits, const std::array<float, k_gamepad_axis_code_cnt> gamepad_axis_values)
        : keys_down_bits(keys_down_bits), mouse_pos(mouse_pos), mouse_buttons_down_bits(mouse_buttons_down_bits), gamepad_glfw_joystick_index(gamepad_glfw_joystick_index), gamepad_buttons_down_bits(gamepad_buttons_down_bits), gamepad_axis_values(gamepad_axis_values)
    {
    }
};

struct s_input_state_pair
{
    s_input_state state;
    s_input_state state_last;

    s_input_state_pair() = default;

    s_input_state_pair(const s_input_state &state, const s_input_state &state_last) : state(state), state_last(state_last)
    {
    }
};

s_input_state gen_input_state(GLFWwindow *const glfw_window);

inline s_input_state gen_blank_input_state()
{
    return s_input_state(0, {}, 0, -1, 0, {});
}

inline bool is_key_down(const ec_key_code key_code, const s_input_state_pair &input_state_pair)
{
    const auto key_bit = static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code);
    return input_state_pair.state.keys_down_bits & key_bit;
}

inline bool is_key_pressed(const ec_key_code key_code, const s_input_state_pair &input_state_pair)
{
    const auto key_bit = static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code);
    return (input_state_pair.state.keys_down_bits & key_bit) && !(input_state_pair.state_last.keys_down_bits & key_bit);
}

inline bool is_key_released(const ec_key_code key_code, const s_input_state_pair &input_state_pair)
{
    const auto key_bit = static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code);
    return !(input_state_pair.state.keys_down_bits & key_bit) && (input_state_pair.state_last.keys_down_bits & key_bit);
}

inline bool is_mouse_button_down(const ec_mouse_button_code button_code, const s_input_state_pair &input_state_pair)
{
    const auto button_bit = static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code);
    return input_state_pair.state.mouse_buttons_down_bits & button_bit;
}

inline bool is_mouse_button_pressed(const ec_mouse_button_code button_code, const s_input_state_pair &input_state_pair)
{
    const auto button_bit = static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code);
    return (input_state_pair.state.mouse_buttons_down_bits & button_bit) && !(input_state_pair.state_last.mouse_buttons_down_bits & button_bit);
}

inline bool is_mouse_button_released(const ec_mouse_button_code button_code, const s_input_state_pair &input_state_pair)
{
    const auto button_bit = static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code);
    return !(input_state_pair.state.mouse_buttons_down_bits & button_bit) && (input_state_pair.state_last.mouse_buttons_down_bits & button_bit);
}

inline bool is_gamepad_button_down(const ec_gamepad_button_code button_code, const s_input_state_pair &input_state_pair)
{
    const auto button_bit = static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code);
    return input_state_pair.state.gamepad_buttons_down_bits & button_bit;
}

inline bool is_gamepad_button_pressed(const ec_gamepad_button_code button_code, const s_input_state_pair &input_state_pair)
{
    const auto button_bit = static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code);
    return (input_state_pair.state.gamepad_buttons_down_bits & button_bit) && !(input_state_pair.state_last.gamepad_buttons_down_bits & button_bit);
}

inline bool is_gamepad_button_released(const ec_gamepad_button_code button_code, const s_input_state_pair &input_state_pair)
{
    const auto button_bit = static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code);
    return !(input_state_pair.state.gamepad_buttons_down_bits & button_bit) && (input_state_pair.state_last.gamepad_buttons_down_bits & button_bit);
}
