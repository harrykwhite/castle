#pragma once

#include <stdexcept>
#include <filesystem>
#include <memory>
#include <castle_common/cc_math.h>
#include "c_utils.h"
#include "c_modding.h"

constexpr int k_asset_group_cnt = 1 + k_max_mod_cnt;

class c_tex_data
{
public:
    c_tex_data() = default;
    ~c_tex_data();

    void load_from_ifs(std::ifstream &ifs, const int tex_cnt);

    inline int get_tex_cnt() const
    {
        return m_tex_cnt;
    }

    inline u_gl_id get_tex_gl_id(const int index) const
    {
        CC_CHECK(index >= 0 && index < m_tex_cnt);
        return m_gl_ids[index];
    }

    inline cc::s_vec_2d_int get_tex_size(const int index) const
    {
        CC_CHECK(index >= 0 && index < m_tex_cnt);
        return m_sizes[index];
    }

private:
    int m_tex_cnt = 0;
    std::unique_ptr<u_gl_id[]> m_gl_ids;
    std::unique_ptr<cc::s_vec_2d_int[]> m_sizes;
};

class c_shader_prog_data
{
public:
    c_shader_prog_data() = default;
    ~c_shader_prog_data();

    void load_from_ifs(std::ifstream &ifs, const int prog_cnt);

    inline int get_prog_cnt() const
    {
        return m_prog_cnt;
    }

    inline u_gl_id get_prog_gl_id(const int index) const
    {
        CC_CHECK(index >= 0 && index < m_prog_cnt);
        return m_gl_ids[index];
    }

private:
    int m_prog_cnt = 0;
    std::unique_ptr<u_gl_id[]> m_gl_ids;
};

class c_assets
{
public:
    bool load_from_file(const std::string &filename);

    inline u_gl_id get_tex_gl_id(const int index) const
    {
        return m_tex_data.get_tex_gl_id(index);
    }

    inline cc::s_vec_2d_int get_tex_size(const int index) const
    {
        return m_tex_data.get_tex_size(index);
    }

    inline u_gl_id get_shader_prog_gl_id(const int index) const
    {
        return m_shader_prog_data.get_prog_gl_id(index);
    }

private:
    c_tex_data m_tex_data;
    c_shader_prog_data m_shader_prog_data;
};
