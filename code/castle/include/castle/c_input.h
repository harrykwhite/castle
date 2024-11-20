#pragma once

#include <array>
#include <GLFW/glfw3.h>
#include <castle_common/cc_math.h>
#include <castle/c_game.h>

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
    int mouse_scroll;

    int gamepad_glfw_joystick_index;
    u_gamepad_buttons_down_bits gamepad_buttons_down_bits;
    std::array<float, k_gamepad_axis_code_cnt> gamepad_axis_values;

    static s_input_state create_blank()
    {
        return {0, {}, 0, 0, -1, 0, {}};
    }

    s_input_state() = default;

    s_input_state(const u_keys_down_bits keys_down_bits, const cc::s_vec_2d mouse_pos, const u_mouse_buttons_down_bits mouse_buttons_down_bits, const int mouse_scroll, const int gamepad_glfw_joystick_index, const u_gamepad_buttons_down_bits gamepad_buttons_down_bits, const std::array<float, k_gamepad_axis_code_cnt> gamepad_axis_values)
        : keys_down_bits(keys_down_bits), mouse_pos(mouse_pos), mouse_buttons_down_bits(mouse_buttons_down_bits), mouse_scroll(mouse_scroll), gamepad_glfw_joystick_index(gamepad_glfw_joystick_index), gamepad_buttons_down_bits(gamepad_buttons_down_bits), gamepad_axis_values(gamepad_axis_values)
    {
    }
};

class c_input_manager
{
public:
    void refresh(GLFWwindow *const glfw_window, const int mouse_scroll);

    inline bool is_key_down(const ec_key_code key_code) const
    {
        const auto key_bit = static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code);
        return m_state.keys_down_bits & key_bit;
    }

    inline bool is_key_pressed(const ec_key_code key_code) const
    {
        const auto key_bit = static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code);
        return (m_state.keys_down_bits & key_bit) && !(m_state_last.keys_down_bits & key_bit);
    }

    inline bool is_key_released(const ec_key_code key_code) const
    {
        const auto key_bit = static_cast<u_keys_down_bits>(1) << static_cast<int>(key_code);
        return !(m_state.keys_down_bits & key_bit) && (m_state_last.keys_down_bits & key_bit);
    }

    inline bool is_mouse_button_down(const ec_mouse_button_code button_code) const
    {
        const auto button_bit = static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code);
        return m_state.mouse_buttons_down_bits & button_bit;
    }

    inline bool is_mouse_button_pressed(const ec_mouse_button_code button_code) const
    {
        const auto button_bit = static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code);
        return (m_state.mouse_buttons_down_bits & button_bit) && !(m_state_last.mouse_buttons_down_bits & button_bit);
    }

    inline bool is_mouse_button_released(const ec_mouse_button_code button_code) const
    {
        const auto button_bit = static_cast<u_mouse_buttons_down_bits>(1) << static_cast<int>(button_code);
        return !(m_state.mouse_buttons_down_bits & button_bit) && (m_state_last.mouse_buttons_down_bits & button_bit);
    }

    inline bool is_gamepad_button_down(const ec_gamepad_button_code button_code) const
    {
        const auto button_bit = static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code);
        return m_state.gamepad_buttons_down_bits & button_bit;
    }

    inline bool is_gamepad_button_pressed(const ec_gamepad_button_code button_code) const
    {
        const auto button_bit = static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code);
        return (m_state.gamepad_buttons_down_bits & button_bit) && !(m_state_last.gamepad_buttons_down_bits & button_bit);
    }

    inline bool is_gamepad_button_released(const ec_gamepad_button_code button_code) const
    {
        const auto button_bit = static_cast<u_gamepad_buttons_down_bits>(1) << static_cast<int>(button_code);
        return !(m_state.gamepad_buttons_down_bits & button_bit) && (m_state_last.gamepad_buttons_down_bits & button_bit);
    }

    inline cc::s_vec_2d get_mouse_pos() const
    {
        return m_state.mouse_pos;
    }

    inline int get_mouse_scroll() const
    {
        return m_state.mouse_scroll;
    }

private:
    s_input_state m_state = s_input_state::create_blank();
    s_input_state m_state_last = s_input_state::create_blank();
};

s_input_state create_input_state(GLFWwindow *const glfw_window, const int mouse_scroll);
