#include <castle/c_game.h>

#include <iostream>
#include <castle/c_input.h>
#include <castle/c_player.h>

static void glfw_window_size_callback(GLFWwindow *const window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

c_game::~c_game()
{
    if (m_glfw_initialized)
    {
        if (m_glfw_window)
        {
            glfwDestroyWindow(m_glfw_window);
        }

        glfwTerminate();
    }
}

void c_game::run()
{
    //
    // Initialisation
    //

    // Initialise GLFW.
    if (!glfwInit())
    {
        std::cout << "ERROR: Failed to initialize GLFW!" << std::endl;
        return;
    }

    m_glfw_initialized = true;
    std::cout << "Successfully initialized GLFW!" << std::endl;

    // Create the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false); // Show the window later once other systems have been set up.

    m_glfw_window = glfwCreateWindow(1280, 720, k_window_title.c_str(), nullptr, nullptr);

    if (!m_glfw_window)
    {
        std::cout << "ERROR: Failed to create a GLFW window!" << std::endl;
        return;
    }

    glfwMakeContextCurrent(m_glfw_window);

    std::cout << "Successfully created a GLFW window!" << std::endl;

    // Set GLFW window callbacks.
    glfwSetWindowSizeCallback(m_glfw_window, glfw_window_size_callback);

    // Initialise OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "ERROR: Failed to initialise OpenGL function pointers!" << std::endl;
    }

    std::cout << "Successfully initialised OpenGL function pointers!" << std::endl;

    // Enable blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load core assets.
    if (!m_assets.load_core_group())
    {
        return;
    }

    std::cout << "Successfully loaded game assets!" << std::endl;

    // Set up the title scene.
    m_scene = make_scene(ec_scene_type::title);

    // Show the window now that things have been set up.
    glfwShowWindow(m_glfw_window);

    //
    // Main Loop
    //
    double frame_time = glfwGetTime();
    double frame_dur_accum = 0.0;

    c_input_manager input_manager;

    while (!glfwWindowShouldClose(m_glfw_window))
    {
        glfwPollEvents();

        const double frame_time_last = frame_time;
        frame_time = glfwGetTime();

        const double frame_dur = calc_valid_frame_dur(frame_time, frame_time_last);
        frame_dur_accum += frame_dur;

        const int tick_count = frame_dur_accum / k_targ_tick_dur;

        if (tick_count > 0)
        {
            cc::s_vec_2d_int window_size;
            glfwGetWindowSize(m_glfw_window, &window_size.x, &window_size.y);

            input_manager.refresh(m_glfw_window);

            // Execute ticks.
            {
                int i = 0;

                do
                {
                    // Run scene tick.
                    bool change_scene = false;
                    ec_scene_type change_scene_type;

                    m_scene->on_tick(input_manager, m_assets, window_size, change_scene, change_scene_type);

                    if (change_scene)
                    {
                        m_scene = make_scene(change_scene_type);
                    }

                    //
                    frame_dur_accum -= k_targ_tick_dur;

                    ++i;
                }
                while (i < tick_count);
            }

            // Render.
            glfwSwapBuffers(m_glfw_window);
            m_scene->render(m_assets, window_size);
        }
    }
}
