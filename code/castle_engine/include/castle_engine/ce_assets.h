#pragma once

#include <cstdio>
#include <castle_common/cc_math.h>
#include "ce_utils.h"
#include "ce_modding.h"

namespace ce
{

struct s_assets
{
    int tex_cnt;
    int shader_prog_cnt;

    void *buf;
    int buf_size;

    int buf_tex_gl_ids_offs;
    int buf_tex_sizes_offs;

    int buf_shader_prog_gl_ids_offs;

    bool tex_init;
    int shader_prog_init_cnt;
};

const char *const k_assets_file_name = "assets.dat";

bool init_assets_with_file(s_assets *const assets, FILE *const fs, char *const err_msg_buf);
void clean_assets(s_assets *const assets);

inline u_gl_id *get_tex_gl_ids(const s_assets *const assets)
{
    return reinterpret_cast<u_gl_id *>(reinterpret_cast<char *>(assets->buf) + assets->buf_tex_gl_ids_offs);
}

inline cc::s_vec_2d_int *get_tex_sizes(const s_assets *const assets)
{
    return reinterpret_cast<cc::s_vec_2d_int *>(reinterpret_cast<char *>(assets->buf) + assets->buf_tex_sizes_offs);
}

inline u_gl_id *get_shader_prog_gl_ids(const s_assets *const assets)
{
    return reinterpret_cast<u_gl_id *>(reinterpret_cast<char *>(assets->buf) + assets->buf_shader_prog_gl_ids_offs);
}

}
