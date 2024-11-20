#pragma once

#include <bitset>
#include <vector>
#include <numeric>
#include <cassert>
#include <castle_common/cc_math.h>
#include "c_assets.h"
#include "c_utils.h"

enum class ec_font_align_hor
{
    left,
    center,
    right
};

enum class ec_font_align_ver
{
    top,
    center,
    bottom
};

struct s_color
{
    static constexpr s_color white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr s_color black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr s_color red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr s_color green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr s_color blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr s_color yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr s_color cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr s_color magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }

    float r, g, b, a;
};

struct s_camera
{
    static constexpr float scale = 2.0f; // TEMP: This will likely be modifiable in an options menu in the future.
    cc::s_vec_2d pos;
};

#if 0
constexpr int k_tex_unit_limit = 32;

struct s_sprite_batch
{
    static constexpr int k_slot_cnt = 1024;
    static constexpr int k_available_slot_stack_size_max = 32;

    u_gl_id vert_array_gl_id;
    u_gl_id vert_buf_gl_id;
    u_gl_id elem_buf_gl_id;

    std::bitset<k_slot_cnt> slot_activity;

    // This is a stack where the indexes of released slots are pushed, so that when a new slot needs to be taken an available index can be taken from the top here.
    int available_slot_stack[k_available_slot_stack_size_max];
    int available_slot_stack_size;

    int slot_tex_units[k_slot_cnt]; // What texture unit each slot is mapped to.

    s_asset_id tex_unit_tex_ids[k_tex_unit_limit]; // What texture ID (the actual texture asset) each unit maps to.
    int tex_unit_ref_cnts[k_tex_unit_limit]; // How many slots are mapped to each unit.
};

struct s_char_batch
{
    u_gl_id vert_array_gl_id;
    u_gl_id vert_buf_gl_id;
    u_gl_id elem_buf_gl_id;

    int active_slot_cnt; // Effectively the text length.
    int slot_cnt;

    s_asset_id font_id;

    cc::s_vec_2d pos;
    float rot;
    s_color blend;
};

struct s_camera_render_layer_init_info
{
    int sprite_batch_cnt;
    int sprite_batch_slot_cnt;
};

struct s_screen_render_layer_init_info
{
    int sprite_batch_cnt;
    int sprite_batch_slot_cnt;

    int char_batch_cnt;
};

struct s_render_layer_batch_type_info
{
    int begin_batch_index;
    int batch_cnt;
};

struct s_render_data
{
    int cam_layer_cnt;
    int screen_layer_cnt;

    int cam_sprite_batch_cnt;
    int screen_sprite_batch_cnt;
    int screen_char_batch_cnt;

    cc::u_byte *buf;

    // The below are pointers into the above buffer.
    s_sprite_batch *buf_cam_sprite_batches;
    s_sprite_batch *buf_screen_sprite_batches;
    s_char_batch *buf_screen_char_batches;
    s_render_layer_batch_type_info *buf_cam_layers_sprite_batch_type_infos;
    s_render_layer_batch_type_info *buf_screen_layers_sprite_batch_type_infos;
    s_render_layer_batch_type_info *buf_screen_layers_char_batch_type_infos;
};

struct s_sprite_batch_slot_key
{
    bool screen;
    int batch_index;
    int slot_index;
};

struct s_char_batch_key
{
    char batch_index : 8;
};

s_sprite_batch gen_sprite_batch();
void dispose_sprite_batch(s_sprite_batch &batch);

s_char_batch gen_char_batch(const int slot_cnt);
void dispose_char_batch(s_char_batch &batch);
void write_to_char_batch(const s_char_batch &batch, const std::string &text, const ec_font_align_hor align_hor, const ec_font_align_ver align_ver, const c_assets &assets);

s_render_data gen_render_data(const std::vector<s_camera_render_layer_init_info> &cam_layers_init_infos, const std::vector<s_screen_render_layer_init_info> &screen_layers_init_infos);
void dispose_render_data(s_render_data &render_data);

void draw_batches(const s_render_data &render_data, const s_camera &cam, const c_assets &assets, const cc::s_vec_2d_i window_size);

s_sprite_batch_slot_key take_any_sprite_batch_slot(const bool screen, const int layer_index, const s_asset_id tex_id, s_render_data &render_data);
void write_to_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, const s_render_data &render_data, const c_assets &assets, const cc::s_vec_2d pos, const cc::s_rect &src_rect, const cc::s_vec_2d origin = {}, const float rot = 0.0f, const cc::s_vec_2d scale = {1.0f, 1.0f}, const s_color &blend = s_color::white());
void clear_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, const s_render_data &render_data);
void release_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, s_render_data &render_data);

inline cc::s_vec_2d cam_to_screen_pos(const cc::s_vec_2d pos, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    return {
        ((pos.x - cam.pos.x) * cam.scale) + (window_size.x / 2.0f),
        ((pos.y - cam.pos.y) * cam.scale) + (window_size.y / 2.0f)
    };
}

inline cc::s_vec_2d screen_to_cam_pos(const cc::s_vec_2d pos, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    return {
        ((pos.x - (window_size.x / 2.0f)) / cam.scale) + cam.pos.x,
        ((pos.y - (window_size.y / 2.0f)) / cam.scale) + cam.pos.y
    };
}

constexpr int k_sprite_quad_shader_prog_vert_cnt = 14;
constexpr int k_char_quad_shader_prog_vert_cnt = 4;
#endif

struct s_render_layer_init_info
{
    int sprite_batch_default_slot_cnt;
};

struct s_sprite_batch_slot_key
{
    int layer_index;
    int layer_batch_index;
    int slot_index;
};

struct s_char_batch_key
{
    int layer_index;
    int layer_batch_index;
};

constexpr int k_tex_unit_limit = 32;

class c_sprite_batch
{
public:
    c_sprite_batch(const int slot_cnt);
    ~c_sprite_batch();










    // Move Constructor
    c_sprite_batch(c_sprite_batch&& other) noexcept
        : m_vert_array_gl_id(other.m_vert_array_gl_id),
          m_vert_buf_gl_id(other.m_vert_buf_gl_id),
          m_elem_buf_gl_id(other.m_elem_buf_gl_id),
          m_slot_cnt(other.m_slot_cnt),
          m_slot_activity(std::move(other.m_slot_activity)),
          m_slot_tex_units(std::move(other.m_slot_tex_units))
    {
        std::copy(std::begin(other.m_tex_unit_tex_ids), std::end(other.m_tex_unit_tex_ids), std::begin(m_tex_unit_tex_ids));
        std::copy(std::begin(other.m_tex_unit_ref_cnts), std::end(other.m_tex_unit_ref_cnts), std::begin(m_tex_unit_ref_cnts));

        // Reset other's OpenGL resource IDs
        other.m_vert_array_gl_id = 0;
        other.m_vert_buf_gl_id = 0;
        other.m_elem_buf_gl_id = 0;
    }

    // Move Assignment Operator
    c_sprite_batch& operator=(c_sprite_batch&& other) noexcept
    {
        if (this != &other)
        {
            // Free current resources if needed (not required here)
            
            // Move data from other
            m_vert_array_gl_id = other.m_vert_array_gl_id;
            m_vert_buf_gl_id = other.m_vert_buf_gl_id;
            m_elem_buf_gl_id = other.m_elem_buf_gl_id;
            m_slot_activity = std::move(other.m_slot_activity);
            const_cast<int&>(m_slot_cnt) = other.m_slot_cnt; // Adjust const with a cast
            const_cast<std::unique_ptr<int[]>&>(m_slot_tex_units) = std::move(other.m_slot_tex_units);

            std::copy(std::begin(other.m_tex_unit_tex_ids), std::end(other.m_tex_unit_tex_ids), std::begin(m_tex_unit_tex_ids));
            std::copy(std::begin(other.m_tex_unit_ref_cnts), std::end(other.m_tex_unit_ref_cnts), std::begin(m_tex_unit_ref_cnts));

            // Reset other's OpenGL resource IDs
            other.m_vert_array_gl_id = 0;
            other.m_vert_buf_gl_id = 0;
            other.m_elem_buf_gl_id = 0;
        }
        return *this;
    }












    // Provide nullptr as the cam argument if not wanting to draw with a camera view matrix.
    void draw(const c_assets &assets, const cc::s_vec_2d_i window_size, const s_camera *const cam) const;

    int take_any_slot(const s_asset_id tex_id);
    void release_slot(const int slot_index);
    void write_to_slot(const int slot_index, const c_assets &assets, const cc::s_vec_2d pos, const cc::s_rect &src_rect, const cc::s_vec_2d origin, const float rot, const cc::s_vec_2d scale, const float alpha) const;
    void clear_slot(const int slot_index) const;

private:
    u_gl_id m_vert_array_gl_id = 0;
    u_gl_id m_vert_buf_gl_id = 0;
    u_gl_id m_elem_buf_gl_id = 0;

    const int m_slot_cnt;
    c_heap_bitset m_slot_activity;
    std::unique_ptr<int[]> m_slot_tex_units; // What texture unit each slot is mapped to.

    s_asset_id m_tex_unit_tex_ids[k_tex_unit_limit] = {}; // What texture ID (the actual texture asset) each unit maps to.
    int m_tex_unit_ref_cnts[k_tex_unit_limit] = {}; // How many slots are mapped to each unit.

    int find_tex_unit_to_use(const s_asset_id tex_id) const;
};

class c_char_batch
{
public:
    cc::s_vec_2d m_pos = {};
    float m_rot = 0.0f;
    s_color m_blend = s_color::white();

    c_char_batch(const int slot_cnt, const s_asset_id font_id);
    ~c_char_batch();





    // Move Constructor
    c_char_batch(c_char_batch&& other) noexcept
        : m_pos(other.m_pos),
          m_rot(other.m_rot),
          m_blend(other.m_blend),
          m_vert_array_gl_id(other.m_vert_array_gl_id),
          m_vert_buf_gl_id(other.m_vert_buf_gl_id),
          m_elem_buf_gl_id(other.m_elem_buf_gl_id),
          m_active_slot_cnt(other.m_active_slot_cnt),
          m_slot_cnt(other.m_slot_cnt), // Const can be directly initialized
          m_font_id(other.m_font_id)    // Const can be directly initialized
    {
        // Reset other's OpenGL resource IDs
        other.m_vert_array_gl_id = 0;
        other.m_vert_buf_gl_id = 0;
        other.m_elem_buf_gl_id = 0;
        other.m_active_slot_cnt = 0;
    }

    // Move Assignment Operator
    c_char_batch& operator=(c_char_batch&& other) noexcept
    {
        if (this != &other)
        {
            // Move non-const members
            m_pos = other.m_pos;
            m_rot = other.m_rot;
            m_blend = other.m_blend;
            m_vert_array_gl_id = other.m_vert_array_gl_id;
            m_vert_buf_gl_id = other.m_vert_buf_gl_id;
            m_elem_buf_gl_id = other.m_elem_buf_gl_id;
            m_active_slot_cnt = other.m_active_slot_cnt;

            // No need to modify const members; they retain the same value

            // Reset other's OpenGL resource IDs
            other.m_vert_array_gl_id = 0;
            other.m_vert_buf_gl_id = 0;
            other.m_elem_buf_gl_id = 0;
            other.m_active_slot_cnt = 0;
        }
        return *this;
    }










    void write(const std::string &text, const c_assets &assets, const ec_font_align_hor align_hor, const ec_font_align_ver align_ver);
    void draw(const c_assets &assets, const cc::s_vec_2d_i window_size) const;

private:
    u_gl_id m_vert_array_gl_id = 0;
    u_gl_id m_vert_buf_gl_id = 0;
    u_gl_id m_elem_buf_gl_id = 0;

    int m_active_slot_cnt = 0; // Effectively the text length (one slot per character).
    const int m_slot_cnt;

    const s_asset_id m_font_id;
};

class c_renderer
{
public:
    c_renderer(const std::vector<s_render_layer_init_info> &&layer_init_infos, const int sprite_batch_cam_layer_cnt);

    void draw(const c_assets &assets, const cc::s_vec_2d_i window_size, const s_camera &cam) const;

    s_sprite_batch_slot_key take_any_sprite_batch_slot(const int layer_index, const s_asset_id tex_id);
    s_char_batch_key add_char_batch_to_layer(const int layer_index, const int slot_cnt, const s_asset_id font_id);

    inline void release_sprite_batch_slot(const s_sprite_batch_slot_key &key)
    {
        const int batch_index = m_layer_infos[key.layer_index].begin_sprite_batch_index + key.layer_batch_index;
        m_sprite_batches[batch_index].release_slot(key.slot_index);
    }

    inline void write_to_sprite_batch_slot(const s_sprite_batch_slot_key &key, const c_assets &assets, const cc::s_vec_2d pos, const cc::s_rect &src_rect, const cc::s_vec_2d origin = {}, const float rot = 0.0f, const cc::s_vec_2d scale = {1.0f, 1.0f}, const float alpha = 1.0f) const
    {
        const int batch_index = m_layer_infos[key.layer_index].begin_sprite_batch_index + key.layer_batch_index;
        m_sprite_batches[batch_index].write_to_slot(key.slot_index, assets, pos, src_rect, origin, rot, scale, alpha);
    }

    inline void clear_sprite_batch_slot(const s_sprite_batch_slot_key &key) const
    {
        const int batch_index = m_layer_infos[key.layer_index].begin_sprite_batch_index + key.layer_batch_index;
        m_sprite_batches[batch_index].clear_slot(key.slot_index);
    }

    inline void write_to_char_batch(const s_char_batch_key key, const std::string &text, const c_assets &assets, const ec_font_align_hor align_hor = ec_font_align_hor::left, const ec_font_align_ver align_ver = ec_font_align_ver::top)
    {
        const int batch_index = m_layer_infos[key.layer_index].begin_sprite_batch_index + key.layer_batch_index;
        m_char_batches[batch_index].write(text, assets, align_hor, align_ver);
    }

private:
    struct s_render_layer_info
    {
        int begin_sprite_batch_index;
        int sprite_batch_cnt;
        int begin_char_batch_index;
        int char_batch_cnt;
    };

    std::vector<c_sprite_batch> m_sprite_batches;
    std::vector<c_char_batch> m_char_batches;

    const std::vector<s_render_layer_init_info> m_layer_init_infos;
    const std::unique_ptr<s_render_layer_info[]> m_layer_infos;

    const int m_sprite_batch_cam_layer_cnt; // Sprite batches 0 through to this minus 1 use a camera-based view matrix.
    
    void add_sprite_batch_to_layer(const int layer_index);

    inline int get_layer_cnt() const
    {
        return m_layer_init_infos.size();
    }
};

inline cc::s_vec_2d cam_to_screen_pos(const cc::s_vec_2d pos, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    return {
        ((pos.x - cam.pos.x) * cam.scale) + (window_size.x / 2.0f),
        ((pos.y - cam.pos.y) * cam.scale) + (window_size.y / 2.0f)
    };
}

inline cc::s_vec_2d screen_to_cam_pos(const cc::s_vec_2d pos, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    return {
        ((pos.x - (window_size.x / 2.0f)) / cam.scale) + cam.pos.x,
        ((pos.y - (window_size.y / 2.0f)) / cam.scale) + cam.pos.y
    };
}

constexpr int k_sprite_quad_shader_prog_vert_cnt = 11;
constexpr int k_char_quad_shader_prog_vert_cnt = 4;
