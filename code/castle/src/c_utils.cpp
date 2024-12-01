#include "c_utils.h"

bool are_all_bits_active(const cc::Byte *const bytes, const int bitCnt)
{
    assert(bitCnt > 0);

    const int byteCnt = bit_to_byte_cnt(bitCnt);

    for (int i = 0; i < byteCnt - 1; ++i)
    {
        if (bytes[i] != 0xFF)
        {
            return false;
        }
    }

    const cc::Byte lastByteMask = (1 << (bitCnt % 8)) - 1;

    if ((bytes[byteCnt - 1] & lastByteMask) != lastByteMask)
    {
        return false;
    }

    return true;
}

int first_active_bit_index(const cc::Byte *const bytes, const int bitCnt)
{
    assert(bitCnt > 0);

    for (int i = 0; i < bitCnt / 8; ++i)
    {
        if (!bytes[i])
        {
            continue;
        }

        for (int j = 0; j < 8; ++j)
        {
            if (bytes[i] & (1 << j))
            {
                return (i * 8) + j;
            }
        }
    }

    const int byteCnt = bit_to_byte_cnt(bitCnt);

    for (int i = 0; i < bitCnt % 8; ++i)
    {
        if (bytes[byteCnt - 1] & (1 << i))
        {
            return ((byteCnt - 1) * 8) + i;
        }
    }

    return -1;
}

int first_inactive_bit_index(const cc::Byte *const bytes, const int bitCnt)
{
    assert(bitCnt > 0);

    for (int i = 0; i < bitCnt / 8; ++i)
    {
        if (bytes[i] == 0xFF)
        {
            continue;
        }

        for (int j = 0; j < 8; ++j)
        {
            if (!(bytes[i] & (1 << j)))
            {
                return (i * 8) + j;
            }
        }
    }

    const int byteCnt = bit_to_byte_cnt(bitCnt);

    for (int i = 0; i < bitCnt % 8; ++i)
    {
        if (!(bytes[byteCnt - 1] & (1 << i)))
        {
            return ((byteCnt - 1) * 8) + i;
        }
    }

    return -1;
}
