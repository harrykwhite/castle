#pragma once

#include <castle_common/cc_math.h>
#include "c_input.h"
#include "c_assets.h"
#include "c_rendering.h"

enum MainMenuRenderLayer
{
    MAIN_MENU_GENERAL_LAYER,

    MAIN_MENU_LAYER_CNT
};

struct MainMenu
{
    Renderer renderer;
    CharBatchKey titleTextCBKey;
    CharBatchKey startTextCBKey;
};

void init_main_menu(MainMenu &menu, cc::MemArena &permMemArena, cc::MemArena &tempMemArena, const AssetGroupManager &assetGroupManager);
void clean_main_menu(MainMenu &menu);
void main_menu_tick(MainMenu &menu, bool &goToWorld, const InputManager &inputManager);
void main_menu_on_window_resize(MainMenu &menu);
