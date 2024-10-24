#pragma once

namespace ce
{

#if 0
// You have sprite batch layers which link to sprite batches. Maybe they can be accessed like a linked list? Jumping from one batch to another in the same layer is not a common operation anyway.
// Ideally support an indefinite number of batches in a layer.
// - Reallocation of a list when resize is needed?

// Ideally don't initialize a sprite batch until you need it. This is what a static array might warrant.

struct s_sprite_batch
{
    u_gl_id vert_array_gl_id;
    u_gl_id vert_buf_gl_id;
    u_gl_id elem_buf_gl_id;
};

struct s_sprite_batch_layer
{
    s_sprite_batch *batches;
    int batch_cnt;
};

struct s_render_data
{
    s_sprite_batch_layer *sprite_batch_layers;
    int sprite_batch_layer_cnt;
};

template<typename T>
T *allocate(const int cnt = 1) {
    assert(cnt >= 1);
    return std::malloc(sizeof(T) * cnt);
}

void take_sprite_batch_slot(s_sprite_batch_layer *const batch_layer) {
    for (int i = 0; i < batch_layer->batch_cnt; ++i) {
        // Do something...
    }

    ++batch_layer->batch_cnt;
    batch_layer->batches = allocate<s_sprite_batch>(batch_layer->batch_cnt);
}











struct s_render_data
{
    s_sprite_batch *sprite_batches; // An array containing the first batches of the layers.
};

// When do you remove a batch? When it is rendering nothing and nothing has been added to it for x seconds. Should slots be reallocated to batches if space is available?
// The batches cannot be stack-like...

struct s_item
{
    s_item *next;
};

static s_item *get_list_item(s_item *const list, const int index)
{
    assert(index >= 0);

    if (!index || !list)
    {
        return list;
    }

    return get_list_item(list->next, index - 1);
}

bool take_item_from_list(const int index, s_item *const list)
{
    // Get a pointer to the item before the item to remove. Set its "next" pointer to the item-to-remove's "next" pointer.
    // Get a pointer to the item to remove. Free it.

    if (index < 1)
    {
        return false;
    }

    const s_item *item_to_remove = get_list_item(list, index);

    for (int i = 0; i < index; ++i)
    {
        if (!item_to_remove->next)
        {
            return false;
        }
        
        item_to_remove = item_to_remove->next;
    }
    

    s_item *item_to_remove_prior = list;
    s_item *item_to_remove = list;

    for (int i = 0; i < index; ++i)
    {
        if (!item_to_remove->next)
        {
            return false;
        }

        item_to_remove = item_to_remove->next;
    }

    if (item_to_remove->next)
    {

    }

    return true;
}

static bool take_sprite_batch_slot(s_sprite_batch *const batch)
{


    // A slot couldn't be taken from this batch, so try the next if there is one.
    if (batch->next)
    {
        return take_sprite_batch_slot(batch->next);
    }

    // The operation failed!
    return false;
}

void take_sprite_batch_slot_key(const int layer_index, s_render_data *const render_data)
{
    const bool slot_take_successful = take_sprite_batch_slot(&render_data->sprite_batches[layer_index]);

}

}

#endif

#if 0

#include <GLFW/glfw3.h>
#include <castle_common/cc_misc.h>
#include "ce_assets.h"
#include "ce_utils.h"

namespace ce
{

enum class ec_sprite_batch_group_id
{
    view,
    screen
};

enum class ec_sprite_batch_slot_key_elem_id
{
    batch_group_index,
    layer_index,
    batch_index,
    slot_index,
    tex_unit_index
};

using t_sprite_batch_slot_key = unsigned int;

constexpr int k_render_layer_limit = 32;
constexpr int k_render_layer_sprite_batch_limit = 16;

constexpr int k_sprite_batch_group_cnt = static_cast<int>(ec_sprite_batch_group_id::screen) + 1;
constexpr int k_sprite_batch_slot_cnt = 4096;
constexpr int k_sprite_batch_tex_unit_cnt = 32;

constexpr int k_sprite_batch_slot_key_elem_cnt = static_cast<int>(ec_sprite_batch_slot_key_elem_id::tex_unit_index) + 1;

constexpr int k_sprite_batch_slot_key_elem_bit_counts[k_sprite_batch_slot_key_elem_cnt] = {
    cc::get_uint_bit_cnt(k_sprite_batch_group_cnt),
    cc::get_uint_bit_cnt(k_render_layer_limit),
    cc::get_uint_bit_cnt(k_render_layer_sprite_batch_limit),
    cc::get_uint_bit_cnt(k_sprite_batch_slot_cnt),
    cc::get_uint_bit_cnt(k_sprite_batch_tex_unit_cnt)
};

struct s_sprite_batch_group
{
    s_bitset<k_render_layer_sprite_batch_limit> batch_activity_bits[k_render_layer_limit];

    u_gl_id vert_array_gl_ids[k_render_layer_limit][k_render_layer_sprite_batch_limit];
    u_gl_id vert_buf_gl_ids[k_render_layer_limit][k_render_layer_sprite_batch_limit];
    u_gl_id elem_buf_gl_ids[k_render_layer_limit][k_render_layer_sprite_batch_limit];
};

void init_sprite_batch_group(s_sprite_batch_group *const batch_group);
void clean_sprite_batch_group(s_sprite_batch_group *const batch_group);

constexpr int get_sprite_batch_slot_key_elem_bit_index(const ec_sprite_batch_slot_key_elem_id elem_id)
{
    int bit_index = 1;

    for (int i = 0; i < static_cast<int>(elem_id); i++)
    {
        bit_index += k_sprite_batch_slot_key_elem_bit_counts[static_cast<int>(elem_id)];
    }

    return bit_index;
}

t_sprite_batch_slot_key create_sprite_batch_slot_key(const ec_sprite_batch_group_id batch_group_id, const int layer_index, const int batch_index, const int slot_index, const int tex_unit_index);

inline bool is_sprite_batch_slot_key_active(const t_sprite_batch_slot_key slot_key)
{
    return slot_key & 1;
}

inline int get_sprite_batch_slot_key_elem(const t_sprite_batch_slot_key slot_key, const ec_sprite_batch_slot_key_elem_id elem_id)
{
    return (slot_key >> get_sprite_batch_slot_key_elem_bit_index(elem_id)) & ((1 << k_sprite_batch_slot_key_elem_bit_counts[static_cast<int>(elem_id)]) - 1);
}

}






















#include <vector>

int main()
{
    ce::Game game("Terraria", 1280, 720);

    try
    {
        game.run();
    }
    catch (Exception exc)
    {
    }

    return 0;
}

namespace ce
{

class Game
{
public:
    Game(const char *const init_window_title) {
        glfw_window = nullptr;
    }

    ~Game() {
        if (glfw_window)
        {
            glfwDestroyWindow(glfw_window);
        }

        glfwTerminate();
    }

    void run()
    {
        if (!glfwInit())
        {
            throw new RuntimeError("Failed to initialize GLFW!");
        }
    }

private:
    GLFWwindow *glfw_window;
};

struct InputState
{
    t_keys_down_bits keys_down_bits;
    t_mouse_buttons_down_bits mouse_buttons_down_bits;
    t_gamepad_buttons_down_bits gamepad_buttons_down_bits;

    cc::s_vec_2d mouse_pos;
    int mouse_scroll;

    int gamepad_glfw_joystick_index;
    float gamepad_axis_values[k_gamepad_axis_code_cnt];
};

class Input
{
public:
    void refresh()
    {
        
    }

    inline bool is_key_down(const ec_key_code key_code)
    {
        return (input_state->keys_down_bits & (static_cast<t_keys_down_bits>(1) << static_cast<int>(key_code))) != 0;
    }

private:
    InputState inputState;
    InputState inputStateLast;
};










class SpriteBatch
{
public:
    bool take_sprite_batch_slot(const int tex_id)
    {
        if (is_full())
        {
            return false;
        }


    }

    bool is_full()
    {
        return false;
    }

private:
    u_gl_id m_vert_array_gl_id = 0;
    u_gl_id m_vert_buf_gl_id = 0;
    u_gl_id m_elem_buf_gl_id = 0;

    uint8_t *m_slot_activity_bits = nullptr;
};

class SpriteBatchLayer
{
public:
    bool take_sprite_batch_slot(const int tex_id)
    {
        for (int i = 0; i < m_batches.size(); ++i)
        {
            if (m_batches[i].take_sprite_batch_slot(tex_id))
            {
                return true;
            }
        }

        m_batches.emplace_back();

        return false;
    }

private:
    std::vector<SpriteBatch> m_batches;
};

class Renderer
{
public:
    bool take_sprite_batch_slot(const int tex_id, const int layer_id)
    {
        return m_sb_layers[layer_id].take_sprite_batch_slot(tex_id);
    }

private:
    std::vector<SpriteBatchLayer> m_sb_layers;
};

struct s_sprite_batch
{
    u_gl_id vert_array_gl_id;
    u_gl_id vert_buf_gl_id;
    u_gl_id elem_buf_gl_id;
};

struct s_sprite_batch_layer
{
    s_sprite_batch *batches;
    int batch_cnt;
};

struct s_renderer
{
    s_sprite_batch_layer *sb_layers;
    int sb_layer_cnt;
};

void take_sprite_batch_slot(const int tex_id, const int layer_id, s_renderer &renderer)
{
    for (int i = 0; i < renderer.sb_layers[layer_id].batch_cnt; ++i)
    {
        
    }
}

void render_sprite_batches() {
}

}

struct s_sprite_batches {
    u_gl_id *vert_array_gl_ids;
    u_gl_id *vert_buf_gl_ids;
    u_gl_id *elem_buf_gl_ids;
    uint32_t *active_tex_units;
};

u_gl_id vert_array_gl_id;
u_gl_id vert_buf_gl_id;
u_gl_id elem_buf_gl_id;
uint32_t active_tex_units;

void take_slot(const int tex_id, const int layer_index) {
    s_sprite_batch batch = batches[layer_index];
    
    while (batch) {
        if (batch is full) {
            if (batch.next) {
                batch = batch.next;
                continue;
            }
            else
            {
                break;
            }
        }
    }
}

// process:
// does this texture map to one of the texture units?
    // approach 1: you have an array and each index is a texture ID. simply check for the texture ID in this array.

// insertion:
// - jump to the first batch of a layer
// - if the batch is full OR it has reached max tex unit limit and this tex unit does not belong there


// update:
//

// removal:
//





#endif

}
