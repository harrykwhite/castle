#include "c_main_menu.h"

static constexpr int ik_textCenterVerOffs = 48;

static RenderLayerCreateInfo render_layer_factory(const int index)
{
    switch (index)
    {
        case MAIN_MENU_GENERAL_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = 1024,
                .charBatchCnt = 8
            };

        default:
            assert(false);
            return {};
    }
}

static cc::Vec2D get_title_text_pos(const cc::Vec2DInt windowSize)
{
    return {windowSize.x / 2.0f, (windowSize.y / 2.0f) - ik_textCenterVerOffs};
}

static cc::Vec2D get_start_text_pos(const cc::Vec2DInt windowSize)
{
    return {windowSize.x / 2.0f, (windowSize.y / 2.0f) + ik_textCenterVerOffs};
}

void init_main_menu(MainMenu &menu, const AssetGroupManager &assetGroupManager, const cc::Vec2DInt windowSize)
{
    init_renderer(menu.renderer, MAIN_MENU_LAYER_CNT, 0, render_layer_factory);

    menu.titleTextCBKey = activate_any_char_batch(menu.renderer, MAIN_MENU_GENERAL_LAYER, 32, make_vanilla_asset_id(cc::EB_GARAMOND_72_VANILLA_FONT), assetGroupManager);
    write_to_char_batch(menu.renderer, menu.titleTextCBKey, "Castle", FONT_HOR_ALIGN_CENTER, FONT_VER_ALIGN_CENTER, assetGroupManager);
    set_char_batch_pos(menu.renderer, menu.titleTextCBKey, get_title_text_pos(windowSize));

    menu.startTextCBKey = activate_any_char_batch(menu.renderer, MAIN_MENU_GENERAL_LAYER, 32, make_vanilla_asset_id(cc::EB_GARAMOND_24_VANILLA_FONT), assetGroupManager);
    write_to_char_batch(menu.renderer, menu.startTextCBKey, "Press [Enter] to Start", FONT_HOR_ALIGN_CENTER, FONT_VER_ALIGN_CENTER, assetGroupManager);
    set_char_batch_pos(menu.renderer, menu.startTextCBKey, get_start_text_pos(windowSize));
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

void main_menu_on_window_resize(MainMenu &menu, const cc::Vec2DInt windowSize)
{
    set_char_batch_pos(menu.renderer, menu.titleTextCBKey, get_title_text_pos(windowSize));
    set_char_batch_pos(menu.renderer, menu.startTextCBKey, get_start_text_pos(windowSize));
}
