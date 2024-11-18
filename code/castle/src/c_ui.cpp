#include <castle/c_ui.h>

s_ui make_ui(s_sprite_batch_collection &sb_collection)
{
    s_ui ui;

    ui.cursor_sb_slot_key = take_any_sprite_batch_slot(static_cast<int>(ec_sprite_batch_layer::cursor), s_asset_id::make_core_tex_id(ec_core_tex::cursor), sb_collection);

    for (int i = 0; i < k_player_inv_hotbar_slot_cnt; ++i)
    {
        ui.player_sb_slot_key[i] = take_any_sprite_batch_slot(static_cast<int>(ec_sprite_batch_layer::player_inv_hotbar), s_asset_id::make_core_tex_id(ec_core_tex::inv_slot), sb_collection);
    }

    return ui;
}

void write_ui_render_data(const s_ui &ui, const s_sprite_batch_collection &sb_collection, const c_assets &assets, const s_input_state_pair &input_state_pair, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    // Write inventory hotbar slots.
    {
        const cc::s_vec_2d_i slot_tex_size = assets.get_tex_size(s_asset_id::make_core_tex_id(ec_core_tex::inv_slot));

        const cc::s_vec_2d hotbar_pos = {(window_size.x / 2.0f), (window_size.y / 10.0f) * 9.0f};
        const float hotbar_slot_scale = 2.0f;
        const float hotbar_slot_gap = (slot_tex_size.x * hotbar_slot_scale) * 1.5f;
        const float hotbar_slot_pos_x_offs = -hotbar_slot_gap * ((k_player_inv_hotbar_slot_cnt - 1) / 2.0f);

        for (int i = 0; i < k_player_inv_hotbar_slot_cnt; ++i)
        {
            const cc::s_vec_2d slot_pos = hotbar_pos + (cc::s_vec_2d {hotbar_slot_pos_x_offs + (i * hotbar_slot_gap), 0.0f});
            write_to_sprite_batch_slot(ui.player_sb_slot_key[i], sb_collection, assets, slot_pos, {0, 0, slot_tex_size.x, slot_tex_size.y}, {0.5f, 0.5f}, 0.0f, {cam.scale, cam.scale});
        }
    }

    // Write cursor.
    {
        const cc::s_vec_2d_i cursor_tex_size = assets.get_tex_size(s_asset_id::make_core_tex_id(ec_core_tex::cursor));
        write_to_sprite_batch_slot(ui.cursor_sb_slot_key, sb_collection, assets, input_state_pair.state.mouse_pos, {0, 0, cursor_tex_size.x, cursor_tex_size.y}, {0.5f, 0.5f}, 0.0f, {cam.scale, cam.scale});
    }
}
