#include <castle/c_ui.h>

s_ui create_ui(c_renderer &renderer)
{
    s_ui ui;

    ui.cursor_sb_slot_key = renderer.take_any_sprite_batch_slot(2, s_asset_id::create_core_tex_id(ec_core_tex::cursor));

    for (int i = 0; i < k_player_inv_hotbar_slot_cnt; ++i)
    {
        ui.player_sb_slot_key[i] = renderer.take_any_sprite_batch_slot(1, s_asset_id::create_core_tex_id(ec_core_tex::inv_slot));
    }

    return ui;
}

void write_ui_render_data(const s_ui &ui, const c_renderer &renderer, const c_assets &assets, const c_input_manager &input_manager, const s_camera &cam, const cc::s_vec_2d_i window_size, const int player_inv_hotbar_slot_selected)
{
    // Write inventory hotbar slots.
    {
        const cc::s_vec_2d_i slot_tex_size = assets.get_tex_size(s_asset_id::create_core_tex_id(ec_core_tex::inv_slot));

        const cc::s_vec_2d hotbar_pos = {(window_size.x / 2.0f), (window_size.y / 10.0f) * 9.0f};
        const float hotbar_slot_scale = 2.0f;
        const float hotbar_slot_gap = (slot_tex_size.x * hotbar_slot_scale) * 1.5f;
        const float hotbar_slot_pos_x_offs = -hotbar_slot_gap * ((k_player_inv_hotbar_slot_cnt - 1) / 2.0f);

        for (int i = 0; i < k_player_inv_hotbar_slot_cnt; ++i)
        {
            const cc::s_vec_2d slot_pos = hotbar_pos + (cc::s_vec_2d {hotbar_slot_pos_x_offs + (i * hotbar_slot_gap), 0.0f});

            renderer.write_to_sprite_batch_slot(
                ui.player_sb_slot_key[i],
                assets,
                slot_pos,
                {0, 0, slot_tex_size.x, slot_tex_size.y},
                {0.5f, 0.5f},
                0.0f,
                {cam.scale, cam.scale},
                1.0f
            );
        }
    }

    // Write cursor.
    {
        const cc::s_vec_2d_i cursor_tex_size = assets.get_tex_size(s_asset_id::create_core_tex_id(ec_core_tex::cursor));
        renderer.write_to_sprite_batch_slot(ui.cursor_sb_slot_key, assets, input_manager.get_mouse_pos(), {0, 0, cursor_tex_size.x, cursor_tex_size.y}, {0.5f, 0.5f}, 0.0f, {cam.scale, cam.scale});
    }
}
