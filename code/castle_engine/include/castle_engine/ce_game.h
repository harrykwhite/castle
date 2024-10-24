#pragma once

#include <GLFW/glfw3.h>
#include "ce_assets.h"
#include "ce_rendering.h"

namespace ce
{

using fp_on_game_init = void (*)();
using fp_on_game_tick = void (*)();
using fp_on_window_resize = void (*)();

struct s_game_core
{
    GLFWwindow *glfw_window;
    s_assets assets;
};

struct s_game_funcs
{
    fp_on_game_init on_init;
    fp_on_game_tick on_tick;
    fp_on_window_resize on_window_resize;
};

const char* const k_window_title = "Castle";
constexpr int k_err_msg_buf_size = 256;

bool run_game(s_game_core *const core, const s_game_funcs &funcs);
void clean_game(s_game_core *const core);

}
