#include "gameUtils.h"
#include <stdlib.h>

void freeGame(Game game) {
    free(game.grid[0]);
    free(game.grid);
    free(game.scores);
}
