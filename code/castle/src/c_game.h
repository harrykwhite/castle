#pragma once

#include <GLFW/glfw3.h>
#include <AL/alc.h>
#include <castle_common/cc_mem.h>
#include "c_modding.h"
#include "c_input.h"
#include "c_utils.h"
#include "c_assets.h"
#include "c_rendering.h"
#include "c_audio.h"
#include "c_main_menu.h"
#include "c_world.h"

using GameCleanupInfoBitset = unsigned short;

enum GameCleanupInfoBits : GameCleanupInfoBitset
{
    PERM_MEM_ARENA_CLEANUP_BIT = 1 << 0,
    TEMP_MEM_ARENA_CLEANUP_BIT = 1 << 1,
    GLFW_CLEANUP_BIT = 1 << 2,
    GLFW_WINDOW_CLEANUP_BIT = 1 << 3,
    AL_DEVICE_CLEANUP_BIT = 1 << 4,
    AL_CONTEXT_CLEANUP_BIT = 1 << 5,
    ASSET_GROUP_MANAGER_CLEANUP_BIT = 1 << 6,
    SHADER_PROGS_CLEANUP_BIT = 1 << 7,
    MAIN_MENU_OR_WORLD_CLEANUP_BIT = 1 << 8
};

struct Game
{
    cc::MemArena permMemArena; // Exists for the duration of the game.
    cc::MemArena tempMemArena; // Reset at the beginning of each frame, generally used just as scratch space.

    GLFWwindow *glfwWindow;

    ALCdevice *alDevice;
    ALCcontext *alContext;

    StaticBitset<k_modLimit> modActivity;

    AssetGroupManager assetGroupManager;
    ShaderProgs shaderProgs;

    InputManager inputManager;
    int glfwCallbackMouseScroll; // An axis representing the scroll wheel movement, updated by the GLFW scroll callback and reset after a new input state is generated.

    SoundManager soundManager;
    MusicManager musicManager;

    MainMenu mainMenu;
    World world; // NOTE: Consider using a union here, as the main menu and world might not ever be simultaneously active.
    bool inWorld;
};

GameCleanupInfoBitset init_game(Game &game);
void run_game_loop(Game &game);
void clean_game(Game &game, const GameCleanupInfoBitset infoBitset);

cc::Vec2DInt get_window_size();
