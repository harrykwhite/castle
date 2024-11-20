#include <castle/c_game.h>

#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>
#include <castle/c_rendering.h>
#include <castle/c_assets.h>
#include <castle/c_input.h>
#include <castle/c_ui.h>

struct s_game_cleanup_info
{
    bool glfw_initialized;
    GLFWwindow *glfw_window;

    c_assets *assets;
};

static void clean_game(const s_game_cleanup_info &info)
{
    std::cout << "Cleaning up..." << std::endl;

    if (info.assets)
    {
        info.assets->dispose_all();
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

void run_game()
{
    s_game_cleanup_info cleanup_info = {};

    //
    // Initialisation
    //
    std::cout << "Initialising..." << std::endl;

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
    glfwSetScrollCallback(glfw_window, glfw_scroll_callback);

    // Hide the cursor.
    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Initialise OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "ERROR: Failed to initialise OpenGL function pointers!" << std::endl;
        clean_game(cleanup_info);
        return;
    }

    // Set up assets.
    c_assets assets;

    if (!assets.load_core_group())
    {
        clean_game(cleanup_info);
        return;
    }

    cleanup_info.assets = &assets;

    // Set up rendering.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    c_renderer renderer({{8}, {40}, {8}}, 1);
    s_camera cam = {};

    // Set up UI.
    s_ui ui = create_ui(renderer);

    // TEMP
    int player_inv_hotbar_slot_selected = 0;

    // Set up input.
    c_input_manager input_manager;

    int glfw_callback_mouse_scroll = 0; // This is an axis representing the scroll wheel movement. It is updated by the GLFW scroll callback and gets reset after a new input state is generated.
    glfwSetWindowUserPointer(glfw_window, &glfw_callback_mouse_scroll);

    // Show the window now that things have been set up.
    glfwShowWindow(glfw_window);

    //
    // Main Loop
    //
    double frame_time = glfwGetTime();
    double frame_dur_accum = 0.0;

    std::cout << "Entering the main loop..." << std::endl;

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
            input_manager.refresh(glfw_window, glfw_callback_mouse_scroll);
            glfw_callback_mouse_scroll = 0;

            // Execute ticks.
            int i = 0;

            do
            {
                //world_tick(world, input_state_pair, renderer, assets, glfw_window_size);

                //player_inv_hotbar_slot_selected = incr_wrapped(player_inv_hotbar_slot_selected, -input_state_pair.state.mouse_scroll, k_player_inv_hotbar_slot_cnt);

                write_ui_render_data(ui, renderer, assets, input_manager, cam, glfw_window_size, player_inv_hotbar_slot_selected);

                frame_dur_accum -= k_targ_tick_dur;
                ++i;
            }
            while (i < tick_cnt);
        }

        // Render.
        glfwSwapBuffers(glfw_window);
        renderer.draw(assets, glfw_window_size, cam);
    }

    clean_game(cleanup_info);
}
