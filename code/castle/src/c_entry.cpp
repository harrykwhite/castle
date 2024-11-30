#include "c_game.h"

int main()
{
    Game game = {};
    const GameCleanupInfoBitset gameCleanupInfoBitset = init_game(game);
    run_game_loop(game);
    clean_game(game, gameCleanupInfoBitset);
}
