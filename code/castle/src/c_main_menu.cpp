#include "c_main_menu.h"

#include "c_game.h"

static constexpr int ik_textCenterVerOffs = 48;

static RenderLayerInitInfo render_layer_factory(const int index)
{
    switch (index)
    {
        case MAIN_MENU_GENERAL_LAYER:
            return {
                .spriteBatchCnt = 0,
                .spriteBatchSlotCnt = 0,
                .charBatchCnt = 8
            };

        default:
            assert(false && "The main menu render layer factory does not support the provided layer index!");
            return {};
    }
}

static cc::Vec2D get_title_text_pos()
{
    return {get_window_size().x / 2.0f, (get_window_size().y / 2.0f) - ik_textCenterVerOffs};
}

static cc::Vec2D get_start_text_pos()
{
    return {get_window_size().x / 2.0f, (get_window_size().y / 2.0f) + ik_textCenterVerOffs};
}

void init_main_menu(MainMenu &menu, const AssetGroupManager &assetGroupManager)
{
    init_renderer(menu.renderer, MAIN_MENU_LAYER_CNT, 0, render_layer_factory);

    menu.titleTextCBKey = activate_any_char_batch(menu.renderer, MAIN_MENU_GENERAL_LAYER, 32, make_vanilla_asset_id(cc::EB_GARAMOND_72_VANILLA_FONT), get_title_text_pos(), assetGroupManager);
    write_to_char_batch(menu.renderer, menu.titleTextCBKey, "Castle", FONT_HOR_ALIGN_CENTER, FONT_VER_ALIGN_CENTER, assetGroupManager);

    menu.startTextCBKey = activate_any_char_batch(menu.renderer, MAIN_MENU_GENERAL_LAYER, 32, make_vanilla_asset_id(cc::EB_GARAMOND_24_VANILLA_FONT), get_start_text_pos(), assetGroupManager);
    write_to_char_batch(menu.renderer, menu.startTextCBKey, "Press [Enter] to Start", FONT_HOR_ALIGN_CENTER, FONT_VER_ALIGN_CENTER, assetGroupManager);
}

void clean_main_menu(MainMenu &menu)
{
    clean_renderer(menu.renderer);
}

void main_menu_tick(MainMenu &menu, bool &goToWorld, const InputManager &inputManager)
{
    if (inputManager.is_key_pressed(KEY_ENTER))
    {
        goToWorld = true;
    }
}

void main_menu_on_window_resize(MainMenu &menu)
{
    menu.renderer.layers[menu.titleTextCBKey.layerIndex].charBatches[menu.titleTextCBKey.batchIndex].pos = get_title_text_pos();
    menu.renderer.layers[menu.startTextCBKey.layerIndex].charBatches[menu.startTextCBKey.batchIndex].pos = get_start_text_pos();
}
