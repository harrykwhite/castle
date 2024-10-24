#include <castle_engine/ce_game.h>

int main()
{
    ce::s_game_core game_core;

    const ce::s_game_funcs game_funcs = {
        nullptr,
        nullptr,
        nullptr
    };

    const bool game_run_successful = ce::run_game(&game_core, game_funcs);

    if (!game_run_successful)
    {
        ce::clean_game(&game_core);
        return 1;
    }
    
    return 0;
}
