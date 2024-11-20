#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <castle_common/cc_math.h>
#include <castle/c_rendering.h>
#include <castle/c_assets.h>
#include <castle/c_input.h>
#include <castle/c_ui.h>

constexpr int k_targ_ticks_per_sec = 60;
constexpr double k_targ_tick_dur = 1.0 / k_targ_ticks_per_sec;

class c_game
{
public:
    ~c_game();
    void run();

private:
    bool m_glfw_initialized = false;
    GLFWwindow *m_glfw_window = nullptr;

    c_assets m_assets;

    c_input_manager m_input_manager;

    static inline double calc_valid_frame_dur(const double frame_time, const double frame_time_last)
    {
        const double dur = frame_time - frame_time_last;
        return dur >= 0.0 && dur <= k_targ_tick_dur * 8.0 ? dur : 0.0;
    }

    static inline void glfw_window_size_callback(GLFWwindow *const window, const int width, const int height)
    {
        glViewport(0, 0, width, height);
    }

    static inline void glfw_scroll_callback(GLFWwindow *const window, const double x_offs, const double y_offs)
    {
        int *const callback_mouse_scroll = static_cast<int *>(glfwGetWindowUserPointer(window));
        *callback_mouse_scroll = y_offs;
    }

    static inline cc::s_vec_2d_i get_glfw_window_size(GLFWwindow *const window)
    {
        cc::s_vec_2d_i size;
        glfwGetWindowSize(window, &size.x, &size.y);
        return size;
    }
};

constexpr int k_gl_version_major = 4;
constexpr int k_gl_version_minor = 1;

constexpr const char *k_window_title = "Castle";
