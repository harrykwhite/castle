#include <castle/c_utils.h>

int c_heap_bitset::get_first_inactive_bit_index() const
{
    for (int i = 0; i < CC_STATIC_ARRAY_LEN(m_bytes); ++i)
    {
        if (m_bytes[i] == 0xFF)
        {
            continue;
        }

        for (int j = 0; j < 8; ++j)
        {
            if (!is_bit_active(j))
            {
                return (i * 8) + j;
            }
        }
    }

    return -1;
}

bool c_heap_bitset::is_full() const
{
    for (int i = 0; i < m_byte_cnt; ++i)
    {
        if (m_bytes[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}

bool c_heap_bitset::is_clear() const
{
    for (int i = 0; i < m_byte_cnt; ++i)
    {
        if (m_bytes[i] != 0)
        {
            return false;
        }
    }

    return true;
}
