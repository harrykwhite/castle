#include <castle/c_game.h>

#include <iostream>

c_game::~c_game()
{
    std::cout << "Cleaning up..." << std::endl;

    if (m_glfw_window)
    {
        glfwDestroyWindow(m_glfw_window);
    }

    if (m_glfw_initialized)
    {
        glfwTerminate();
    }
}

void c_game::run()
{
    //
    // Initialisation
    //
    std::cout << "Initialising..." << std::endl;

    // Initialise GLFW.
    if (!glfwInit())
    {
        std::cout << "ERROR: Failed to initialise GLFW!" << std::endl;
        return;
    }

    m_glfw_initialized = true;

    // Create the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, k_gl_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, k_gl_version_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false); // Show the window later once other systems have been set up.

    m_glfw_window = glfwCreateWindow(1280, 720, k_window_title, nullptr, nullptr);

    if (!m_glfw_window)
    {
        std::cout << "ERROR: Failed to create a GLFW window!" << std::endl;
        return;
    }

    glfwMakeContextCurrent(m_glfw_window);

    // Set GLFW window callbacks.
    glfwSetWindowSizeCallback(m_glfw_window, glfw_window_size_callback);
    glfwSetScrollCallback(m_glfw_window, glfw_scroll_callback);

    // Hide the cursor.
    glfwSetInputMode(m_glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Initialise OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "ERROR: Failed to initialise OpenGL function pointers!" << std::endl;
        return;
    }

    // Set up assets.
    if (!m_assets.load_core_group())
    {
        return;
    }

    // Enable blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up input.
    int glfw_callback_mouse_scroll = 0; // This is an axis representing the scroll wheel movement. It is updated by the GLFW scroll callback and gets reset after a new input state is generated.
    glfwSetWindowUserPointer(m_glfw_window, &glfw_callback_mouse_scroll);

    // Show the window now that things have been set up.
    glfwShowWindow(m_glfw_window);

    //
    // Main Loop
    //
    double frame_time = glfwGetTime();
    double frame_dur_accum = 0.0;

    std::cout << "Entering the main loop..." << std::endl;

    while (!glfwWindowShouldClose(m_glfw_window))
    {
        glfwPollEvents();

        const double frame_time_last = frame_time;
        frame_time = glfwGetTime();

        const double frame_dur = calc_valid_frame_dur(frame_time, frame_time_last);
        frame_dur_accum += frame_dur;

        const int tick_cnt = frame_dur_accum / k_targ_tick_dur;

        const cc::s_vec_2d_i glfw_window_size = get_glfw_window_size(m_glfw_window);

        if (tick_cnt > 0)
        {
            m_input_manager.refresh(m_glfw_window, glfw_callback_mouse_scroll);
            glfw_callback_mouse_scroll = 0;

            // Execute ticks.
            int i = 0;

            do
            {
                frame_dur_accum -= k_targ_tick_dur;
                ++i;
            }
            while (i < tick_cnt);
        }

        // Render.
        glfwSwapBuffers(m_glfw_window);
    }
}
