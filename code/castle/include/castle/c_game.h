#pragma once

void run_game();

constexpr int k_gl_version_major = 4;
constexpr int k_gl_version_minor = 1;

constexpr const char *k_window_title = "Castle";

constexpr int k_targ_ticks_per_sec = 60;
constexpr double k_targ_tick_dur = 1.0 / k_targ_ticks_per_sec;
