#include <castle/c_player.h>

c_player_ent::c_player_ent(c_sprite_batch_layer &sb_layer)
{
    m_sb_slot_key = sb_layer.take_any_available_slot(s_asset_id::make_core_tex_id(ec_core_tex::player));
}

void c_player_ent::proc_movement(const c_input_manager &input_manager)
{
    const cc::s_vec_2d move_axis = {
        input_manager.is_key_down(ec_key_code::d) - input_manager.is_key_down(ec_key_code::a),
        input_manager.is_key_down(ec_key_code::s) - input_manager.is_key_down(ec_key_code::w)
    };

    m_vel = move_axis * k_player_move_spd;

    m_pos += m_vel;
}

void c_player_ent::rewrite_render_data(c_sprite_batch_layer &sb_layer, const c_assets &assets)
{
    s_sprite_batch_slot_write_data write_data = {};
    write_data.pos = m_pos;
    write_data.scale = {1.0f, 1.0f};
    write_data.origin = {0.5f, 0.5f};
    write_data.src_rect.width = 32.0f;
    write_data.src_rect.height = 32.0f;
    write_data.blend = s_color::make_white();

    sb_layer.write_to_slot(m_sb_slot_key, write_data, assets);
}
