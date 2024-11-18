#include <castle/c_ui.h>

s_ui make_ui(s_sprite_batch_collection &sb_collection)
{
    return {
        take_any_sprite_batch_slot(static_cast<int>(ec_sprite_batch_layer::cursor), s_asset_id::make_core_tex_id(ec_core_tex::cursor), sb_collection)
    };
}

void write_ui_render_data(const s_ui &ui, const s_sprite_batch_collection &sb_collection, const c_assets &assets, const s_input_state_pair &input_state_pair, const s_camera &cam)
{
    const cc::s_vec_2d_i tex_size = assets.get_tex_size(s_asset_id::make_core_tex_id(ec_core_tex::cursor));
    write_to_sprite_batch_slot(ui.cursor_sb_slot_key, sb_collection, assets, input_state_pair.state.mouse_pos, {0, 0, 4, 4}, {0.5f, 0.5f}, 0.0f, {cam.scale, cam.scale});
}
