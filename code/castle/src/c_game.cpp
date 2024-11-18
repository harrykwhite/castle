#include <castle/c_game.h>

#include <iostream>
#include <GLFW/glfw3.h>
#include <castle/c_rendering.h>
#include <castle/c_assets.h>
#include <castle/c_input.h>
#include <castle/c_world.h>

struct s_game_cleanup_info
{
    bool glfw_initialized;
    GLFWwindow *glfw_window;

    s_sprite_batch_collection *sprite_batch_collection;

    c_assets *assets;
};

static void clean_game(const s_game_cleanup_info &info)
{
    if (info.assets)
    {
        info.assets->dispose_all();
    }

    if (info.sprite_batch_collection)
    {
        dispose_sprite_batch_collection(*info.sprite_batch_collection);
    }

    if (info.glfw_window)
    {
        glfwDestroyWindow(info.glfw_window);
    }

    if (info.glfw_initialized)
    {
        glfwTerminate();
    }
}

static inline double calc_valid_frame_dur(const double frame_time, const double frame_time_last)
{
    const double dur = frame_time - frame_time_last;
    return dur >= 0.0 && dur <= k_targ_tick_dur * 8.0 ? dur : 0.0;
}

static inline void glfw_window_size_callback(GLFWwindow *const window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

static inline cc::s_vec_2d_i get_glfw_window_size(GLFWwindow *const window)
{
    cc::s_vec_2d_i size;
    glfwGetWindowSize(window, &size.x, &size.y);
    return size;
}

void run_game()
{
    //
    // Initialisation
    //
    s_game_cleanup_info cleanup_info = {};

    // Initialise GLFW.
    if (!glfwInit())
    {
        std::cout << "ERROR: Failed to initialise GLFW!" << std::endl;
        clean_game(cleanup_info);
        return;
    }

    cleanup_info.glfw_initialized = true;

    // Create the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, k_gl_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, k_gl_version_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false); // Show the window later once other systems have been set up.

    GLFWwindow *const glfw_window = glfwCreateWindow(1280, 720, k_window_title, nullptr, nullptr);

    if (!glfw_window)
    {
        std::cout << "ERROR: Failed to create a GLFW window!" << std::endl;
        clean_game(cleanup_info);
        return;
    }

    glfwMakeContextCurrent(glfw_window);

    // Set GLFW window callbacks.
    glfwSetWindowSizeCallback(glfw_window, glfw_window_size_callback);

    // Initialise OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "ERROR: Failed to initialise OpenGL function pointers!" << std::endl;
        clean_game(cleanup_info);
        return;
    }

    // Enable blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up assets.
    c_assets assets;

    if (!assets.load_core_group())
    {
        clean_game(cleanup_info);
        return;
    }

    cleanup_info.assets = &assets;

    // Set up sprite batch collection.
    s_sprite_batch_collection sprite_batch_collection = make_sprite_batch_collection({1, 8, 1}, 2);

    // Create the game world.
    s_world world = make_world(sprite_batch_collection, assets);

    // Show the window now that things have been set up.
    glfwShowWindow(glfw_window);

    //
    // Main Loop
    //
    double frame_time = glfwGetTime();
    double frame_dur_accum = 0.0;

    s_input_state_pair input_state_pair(gen_blank_input_state(), gen_blank_input_state());

    while (!glfwWindowShouldClose(glfw_window))
    {
        glfwPollEvents();

        const double frame_time_last = frame_time;
        frame_time = glfwGetTime();

        const double frame_dur = calc_valid_frame_dur(frame_time, frame_time_last);
        frame_dur_accum += frame_dur;

        const int tick_cnt = frame_dur_accum / k_targ_tick_dur;

        const cc::s_vec_2d_i glfw_window_size = get_glfw_window_size(glfw_window);

        if (tick_cnt > 0)
        {
            input_state_pair = {gen_input_state(glfw_window), input_state_pair.state_last};

            // Execute ticks.
            int i = 0;

            do
            {
                world_tick(world, input_state_pair, sprite_batch_collection, assets, glfw_window_size);

                frame_dur_accum -= k_targ_tick_dur;
                ++i;
            }
            while (i < tick_cnt);
        }

        // Render.
        glfwSwapBuffers(glfw_window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        render_sprite_batches(sprite_batch_collection, world.cam, assets, glfw_window_size);
    }

    clean_game(cleanup_info);
}
