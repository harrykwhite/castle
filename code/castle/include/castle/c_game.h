#pragma once

#include <GLFW/glfw3.h>
#include "c_assets.h"
#include "c_rendering.h"
#include "c_scene.h"

const std::string k_window_title = "Castle";

constexpr int k_targ_ticks_per_sec = 60;
constexpr double k_targ_tick_dur = 1.0 / k_targ_ticks_per_sec;

class c_game
{
public:
    c_game() = default;
    ~c_game();

    void run();

private:
    bool m_glfw_initialized = false;
    GLFWwindow *m_glfw_window = nullptr;

    c_assets m_assets;

    std::unique_ptr<c_scene> m_scene;

    static inline double calc_valid_frame_dur(const double frame_time, const double frame_time_last)
    {
        const double dur = frame_time - frame_time_last;
        return dur >= 0.0 && dur <= k_targ_tick_dur * 8.0 ? dur : 0.0;
    }
};
