#pragma once

#include "cc_math.h"

namespace cc
{

constexpr s_vec_2d_i k_tex_size_limit = {2048, 2048};
constexpr int k_tex_channel_cnt = 4;

constexpr int k_font_char_range_begin = 32;
constexpr int k_font_char_range_size = 95;
constexpr int k_font_tex_channel_cnt = 4;

struct s_font_char_data
{
    int hor_offsets[k_font_char_range_size];
    int ver_offsets[k_font_char_range_size];
    int hor_advances[k_font_char_range_size];

    cc::s_rect src_rects[k_font_char_range_size];

    int kernings[k_font_char_range_size * k_font_char_range_size];
};

struct s_font_data
{
    int line_height;
    s_font_char_data chars;
    cc::s_vec_2d_i tex_size;
};

}
