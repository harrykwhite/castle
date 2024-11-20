#pragma once

#include <bitset>
#include "c_utils.h"

constexpr int k_mod_limit = 127;

struct s_mods_state
{
    std::bitset<k_mod_limit> mod_activity;
};
