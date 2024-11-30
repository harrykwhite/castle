#include "c_game.h"

#include <castle_common/cc_debugging.h>

cc::MemArena g_permMemArena;
cc::MemArena g_tempMemArena;

static inline double calc_valid_frame_dur(const double frameTime, const double frameTimeLast)
{
    const double dur = frameTime - frameTimeLast;
    return dur >= 0.0 && dur <= Game::k_targTickDur * 8.0 ? dur : 0.0;
}

static inline cc::Vec2DInt get_glfw_window_size(GLFWwindow *const window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return {width, height};
}

static inline void glfw_scroll_callback(GLFWwindow *const window, const double xOffs, const double yOffs)
{
    int *const scroll = static_cast<int *>(glfwGetWindowUserPointer(window));
    *scroll = static_cast<int>(yOffs);
}

GameCleanupInfoBitset init_game(Game &game)
{
    GameCleanupInfoBitset cleanupInfoBitset = 0;

    cc::log("Initialising...");

    // Initialise the memory arenas.
    if (!cc::init_mem_arena(g_permMemArena, gk_permMemArenaSize))
    {
        cc::log_error("Failed to initialise the permanent memory arena.");
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= PERM_MEM_ARENA_CLEANUP_BIT;

    if (!cc::init_mem_arena(g_tempMemArena, gk_tempMemArenaSize))
    {
        cc::log_error("Failed to initialise the temporary memory arena.");
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= TEMP_MEM_ARENA_CLEANUP_BIT;

    // Initialise GLFW.
    if (!glfwInit())
    {
        cc::log_error("Failed to initialise GLFW.");
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= GLFW_CLEANUP_BIT;

    // Create the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gk_glVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gk_glVersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false); // Show the window later once other things have been set up.

    game.glfwWindow = glfwCreateWindow(1280, 720, gk_windowTitle, nullptr, nullptr); // TEMP: Initial window size will be determined dynamically later.

    if (!game.glfwWindow)
    {
        cc::log_error("Failed to create a GLFW window.");
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= GLFW_WINDOW_CLEANUP_BIT;

    glfwMakeContextCurrent(game.glfwWindow);

    // Initialise OpenGL function pointers.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        cc::log_error("Failed to initialise OpenGL function pointers.");
        return cleanupInfoBitset;
    }

    // Enable blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Open a playback device for OpenAL.
    game.alDevice = alcOpenDevice(nullptr);

    if (!game.alDevice)
    {
        cc::log_error("Failed to open an OpenAL device.");
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= AL_DEVICE_CLEANUP_BIT;

    // Create an OpenAL context.
    game.alContext = alcCreateContext(game.alDevice, nullptr);

    if (!game.alContext)
    {
        cc::log_error("Failed to create an OpenAL context.");
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= AL_CONTEXT_CLEANUP_BIT;

    alcMakeContextCurrent(game.alContext);

    // Load assets.
    if (!game.assetGroupManager.init())
    {
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= ASSET_GROUP_MANAGER_CLEANUP_BIT;

    if (!load_shader_progs(game.shaderProgGLIDs))
    {
        return cleanupInfoBitset;
    }

    cleanupInfoBitset |= SHADER_PROGS_CLEANUP_BIT;

    // Set up input.
    glfwSetWindowUserPointer(game.glfwWindow, &game.glfwCallbackMouseScroll);
    glfwSetScrollCallback(game.glfwWindow, glfw_scroll_callback);

    // Initialise the main menu.
    init_main_menu(game.mainMenu, game.assetGroupManager, get_glfw_window_size(game.glfwWindow));

    // Show the window now that things have been set up.
    glfwShowWindow(game.glfwWindow);

    return cleanupInfoBitset;
}

void run_game_loop(Game &game)
{
    double frameTime = glfwGetTime();
    double frameDurAccum = 0.0;

    cc::log("Entering the game loop...");

    while (!glfwWindowShouldClose(game.glfwWindow))
    {
        cc::clear_mem_arena(g_tempMemArena);

        const cc::Vec2DInt windowSizeBeforePoll = get_glfw_window_size(game.glfwWindow);

        glfwPollEvents();

        const cc::Vec2DInt windowSize = get_glfw_window_size(game.glfwWindow);

        if (windowSize != windowSizeBeforePoll)
        {
            // A change in window size has been detected; resize the viewport.
            glViewport(0, 0, windowSize.x, windowSize.y);
        }

        const double frameTimeLast = frameTime;
        frameTime = glfwGetTime();

        const double frameDur = calc_valid_frame_dur(frameTime, frameTimeLast);
        frameDurAccum += frameDur;

        const int tickCnt = frameDurAccum / Game::k_targTickDur;

        if (tickCnt > 0)
        {
            // Update input.
            game.inputManager.refresh_states(game.glfwWindow, game.glfwCallbackMouseScroll);
            game.glfwCallbackMouseScroll = 0;

            // Update audio.
            game.soundManager.handle_auto_release_srcs();
            game.musicManager.refresh_src_bufs(game.assetGroupManager);

            // Execute ticks.
            int i = 0;

            do
            {
                if (game.inWorld)
                {
                    // Execute world tick.
                    world_tick(game.world, game.inputManager, game.assetGroupManager, windowSize);
                }
                else
                {
                    // Execute main menu tick.
                    bool goToWorld = false;

                    main_menu_tick(game.mainMenu, goToWorld, game.inputManager);

                    if (goToWorld)
                    {
                        clean_main_menu(game.mainMenu);
                        game.inWorld = true;
                        init_world(game.world, game.assetGroupManager);
                    }
                }

                frameDurAccum -= Game::k_targTickDur;
                ++i;
            }
            while (i < tickCnt);
        }

        // Render.
        glfwSwapBuffers(game.glfwWindow);
        
        if (game.inWorld)
        {
            draw_render_layers(game.world.renderer, Color::make_green(), game.assetGroupManager, game.shaderProgGLIDs, &game.world.cam, windowSize);
        }
        else
        {
            draw_render_layers(game.mainMenu.renderer, Color::make_black(), game.assetGroupManager, game.shaderProgGLIDs, nullptr, windowSize);
        }
    }
}

void clean_game(Game &game, const GameCleanupInfoBitset infoBitset)
{
    cc::log("Cleaning up...");

    if (infoBitset & MAIN_MENU_OR_WORLD_CLEANUP_BIT)
    {
        if (game.inWorld)
        {
            clean_world(game.world);
        }
        else
        {
            clean_main_menu(game.mainMenu);
        }
    }

    game.musicManager.clean();
    game.soundManager.clean();

    if (infoBitset & SHADER_PROGS_CLEANUP_BIT)
    {
        clean_shader_progs(game.shaderProgGLIDs);
    }

    if (infoBitset & ASSET_GROUP_MANAGER_CLEANUP_BIT)
    {
        game.assetGroupManager.clean();
    }

    if (infoBitset & AL_CONTEXT_CLEANUP_BIT)
    {
        alcDestroyContext(game.alContext);
    }

    if (infoBitset & AL_DEVICE_CLEANUP_BIT)
    {
        alcCloseDevice(game.alDevice);
    }

    if (infoBitset & GLFW_WINDOW_CLEANUP_BIT)
    {
        glfwDestroyWindow(game.glfwWindow);
    }

    if (infoBitset & GLFW_CLEANUP_BIT)
    {
        glfwTerminate();
    }

    if (infoBitset & TEMP_MEM_ARENA_CLEANUP_BIT)
    {
        cc::clean_mem_arena(g_tempMemArena);
    }

    if (infoBitset & PERM_MEM_ARENA_CLEANUP_BIT)
    {
        cc::clean_mem_arena(g_permMemArena);
    }

    game = {};
}
