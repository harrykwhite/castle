#pragma once

#include <string>
#include <stdexcept>
#include <filesystem>
#include <memory>
#include <cassert>
#include <castle_common/cc_math.h>
#include <castle_common/cc_io.h>
#include "c_utils.h"
#include "c_modding.h"

const std::string k_core_assets_file_name = "assets.dat";
constexpr int k_asset_group_cnt = 1 + k_mod_limit;

enum class ec_core_tex
{
    player,
    dirt_tile,
    stone_tile,
    cursor
};

enum class ec_core_shader_prog
{
    sprite_quad
};

struct s_asset_group
{
    const u_byte *buf;
    const u_gl_id *buf_tex_gl_ids;
    const cc::s_vec_2d_i *buf_tex_sizes;
    const u_gl_id *buf_shader_prog_gl_ids;

    int tex_cnt;
    int shader_prog_cnt;
};

struct s_asset_id
{
    int group_index;
    int asset_index;

    static constexpr s_asset_id make_core_tex_id(const ec_core_tex tex)
    {
        return {0, static_cast<int>(tex)};
    }

    static constexpr s_asset_id make_core_shader_prog_id(const ec_core_shader_prog prog)
    {
        return {0, static_cast<int>(prog)};
    }

    bool operator==(const s_asset_id &other) const
    {
        return group_index == other.group_index && asset_index == other.asset_index;
    }
};

class c_assets
{
public:
    bool load_core_group();
    void load_and_dispose_mod_groups(const s_mods_state &mods_state);
    void dispose_all();

    inline u_gl_id get_tex_gl_id(const s_asset_id id) const
    {
        assert(id.group_index >= 0 && id.group_index < k_asset_group_cnt);
        assert(id.asset_index >= 0 && id.asset_index < m_groups[id.group_index].tex_cnt);
        return m_groups[id.group_index].buf_tex_gl_ids[id.asset_index];
    }

    inline cc::s_vec_2d_i get_tex_size(const s_asset_id id) const
    {
        assert(id.group_index >= 0 && id.group_index < k_asset_group_cnt);
        assert(id.asset_index >= 0 && id.asset_index < m_groups[id.group_index].tex_cnt);
        return m_groups[id.group_index].buf_tex_sizes[id.asset_index];
    }

    inline u_gl_id get_shader_prog_gl_id(const s_asset_id id) const
    {
        assert(id.group_index >= 0 && id.group_index < k_asset_group_cnt);
        assert(id.asset_index >= 0 && id.asset_index < m_groups[id.group_index].shader_prog_cnt);
        return m_groups[id.group_index].buf_shader_prog_gl_ids[id.asset_index];
    }

private:
    s_asset_group m_groups[k_asset_group_cnt] = {};
    c_bitset<k_asset_group_cnt> m_group_activity;

    void dispose_group(const int index);
    bool load_mod_group(const int mod_index, std::ifstream &ifs, const int tex_cnt, const int shader_prog_cnt);
};

s_asset_group make_asset_group(std::ifstream &ifs, const int tex_cnt, const int shader_prog_cnt);
s_asset_group make_core_asset_group(bool &err);
