#include <castle_engine/ce_game.h>

#include <cstdlib>
#include <string>
#include <stdexcept>
#include <castle_engine/ce_input.h>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_debugging.h>
#include <glad/glad.h>

namespace ce
{

constexpr int k_targ_ticks_per_sec = 60;
constexpr double k_targ_tick_dur = 1.0 / k_targ_ticks_per_sec;

static double calc_valid_frame_dur(const double frame_time, const double frame_time_last)
{
    const double dur = frame_time - frame_time_last;
    return dur >= 0.0 && dur <= k_targ_tick_dur * 8.0 ? dur : 0.0;
}

static void run_main_loop(s_game_core *const game_core, const s_game_funcs &game_funcs)
{
    double frame_time = glfwGetTime();
    double frame_dur_accum = 0.0;

    s_input_state input_state = {};

    while (!glfwWindowShouldClose(game_core->glfw_window))
    {
        glfwPollEvents();

        const double frame_time_last = frame_time;
        frame_time = glfwGetTime();

        const double frame_dur = calc_valid_frame_dur(frame_time, frame_time_last);
        frame_dur_accum += frame_dur;

        const int tick_count = frame_dur_accum / k_targ_tick_dur;

        if (tick_count > 0)
        {
            const s_input_state input_state_last = input_state;
            refresh_input_state(&input_state, game_core->glfw_window);

            {
                int i = 0;

                do
                {
                    //game_funcs.on_tick();

                    frame_dur_accum -= k_targ_tick_dur;

                    ++i;
                }
                while (i < tick_count);
            }

            glfwSwapBuffers(game_core->glfw_window);
        }
    }
}

bool run_game(s_game_core *const core, const s_game_funcs &funcs)
{
    core->glfw_window = nullptr;

    // Initialize GLFW.
    if (!glfwInit())
    {
        cc::log_error("Failed to initialize GLFW!");
        return false;
    }

    cc::log("Successfully initialized GLFW!");

    // Create the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false); // Show the window later once other systems have been set up.

    core->glfw_window = glfwCreateWindow(1280, 720, k_window_title, nullptr, nullptr);

    if (!core->glfw_window)
    {
        cc::log_error("Failed to create a GLFW window!");
        return false;
    }

    glfwMakeContextCurrent(core->glfw_window);

    cc::log("Successfully created a GLFW window!");

    // Initialize OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        cc::log_error("Failed to initialize OpenGL function pointers!");
    }

    cc::log("Successfully initialized OpenGL function pointers!");

    // Initialize the error message buffer.
    char err_msg_buf[k_err_msg_buf_size];
    err_msg_buf[k_err_msg_buf_size - 1] = '\0';

    // Initialize assets.
    {
        FILE *const assets_file_fs = std::fopen(k_assets_file_name, "rb");

        if (!assets_file_fs)
        {
            cc::log_error("Failed to open \"%s\"!", k_assets_file_name);
            return false;
        }

        cc::log("Successfully opened \"%s\"!", k_assets_file_name);

        const bool init_successful = init_assets_with_file(&core->assets, assets_file_fs, err_msg_buf);

        std::fclose(assets_file_fs);

        if (!init_successful)
        {
            cc::log_error(err_msg_buf);
            return false;
        }
    }

    // Call the game initialisation function.
    //funcs.on_init();

    // Show the window now that things have been set up.
    glfwShowWindow(core->glfw_window);

    // Run the main loop.
    run_main_loop(core, funcs);

    return true;
}

void clean_game(s_game_core *const game_core)
{
    clean_assets(&game_core->assets);

    if (game_core->glfw_window)
    {
        glfwDestroyWindow(game_core->glfw_window);
    }

    glfwTerminate();
}

}
