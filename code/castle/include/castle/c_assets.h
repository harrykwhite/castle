#pragma once

#include <string>
#include <stdexcept>
#include <filesystem>
#include <memory>
#include <castle_common/cc_math.h>
#include "c_utils.h"
#include "c_modding.h"

const std::string k_core_assets_file_name = "assets.dat";
constexpr int k_asset_group_cnt = 1 + k_max_mod_cnt;

enum class ec_core_tex
{
    player,
    dirt_tile,
    stone_tile
};

enum class ec_core_shader_prog
{
    sprite_quad
};

class c_asset_group
{
public:
    c_asset_group() = default;
    ~c_asset_group();

    void load_from_ifs(std::ifstream &ifs, const int tex_cnt, const int shader_prog_cnt);

    inline int get_tex_cnt() const
    {
        return m_tex_cnt;
    }

    inline int get_shader_prog_cnt() const
    {
        return m_shader_prog_cnt;
    }

    inline u_gl_id get_tex_gl_id(const int index) const
    {
        CC_CHECK(index >= 0 && index < m_tex_cnt);
        return m_tex_gl_ids[index];
    }

    inline cc::s_vec_2d_int get_tex_size(const int index) const
    {
        CC_CHECK(index >= 0 && index < m_tex_cnt);
        return m_tex_sizes[index];
    }

    inline u_gl_id get_shader_prog_gl_id(const int index) const
    {
        CC_CHECK(index >= 0 && index < m_shader_prog_cnt);
        return m_shader_prog_gl_ids[index];
    }

private:
    int m_tex_cnt = 0;
    int m_shader_prog_cnt = 0;

    std::unique_ptr<u_gl_id[]> m_tex_gl_ids;
    std::unique_ptr<cc::s_vec_2d_int[]> m_tex_sizes;

    std::unique_ptr<u_gl_id[]> m_shader_prog_gl_ids;
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

    inline void load_mod_group_from_ifs(const int group_index, std::ifstream &ifs, const int tex_cnt, const int shader_prog_cnt)
    {
        CC_CHECK(group_index >= 0 && group_index < k_asset_group_cnt);
        m_groups[group_index].load_from_ifs(ifs, tex_cnt, shader_prog_cnt);
    }

    inline int get_group_tex_cnt(const int index) const
    {
        CC_CHECK(index >= 0 && index < k_asset_group_cnt);
        return m_groups[index].get_tex_cnt();
    }

    inline int get_group_shader_prog_cnt(const int index) const
    {
        CC_CHECK(index >= 0 && index < k_asset_group_cnt);
        return m_groups[index].get_shader_prog_cnt();
    }

    inline u_gl_id get_tex_gl_id(const s_asset_id &id) const
    {
        CC_CHECK(id.group_index >= 0 && id.group_index < k_asset_group_cnt);
        return m_groups[id.group_index].get_tex_gl_id(id.asset_index);
    }

    inline cc::s_vec_2d_int get_tex_size(const s_asset_id &id) const
    {
        CC_CHECK(id.group_index >= 0 && id.group_index < k_asset_group_cnt);
        return m_groups[id.group_index].get_tex_size(id.asset_index);
    }

    inline u_gl_id get_shader_prog_gl_id(const s_asset_id &id) const
    {
        CC_CHECK(id.group_index >= 0 && id.group_index < k_asset_group_cnt);
        return m_groups[id.group_index].get_shader_prog_gl_id(id.asset_index);
    }

private:
    c_asset_group m_groups[k_asset_group_cnt];
};
