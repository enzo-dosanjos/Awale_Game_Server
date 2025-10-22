#include "gameLogic.h"
#include <stdlib.h>


int** initGrid () {
    int** grid = malloc(NUM_HOUSES * NUM_PLAYERS * sizeof(int));
    int numSeedsPerHouse = NUM_SEEDS / NUM_HOUSES;
    
    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < NUM_HOUSES; j++) {
            grid[i][j] = numSeedsPerHouse;
        }
    }

    return grid;
}



int play (Game game, Move move) {
    // check legalite

    int houseInd = move.numPlayer * NUM_HOUSES + move.houseNum;
    int numSeeds = game.grid[move.numPlayer][move.houseNum];

    game.grid[houseInd] = 0;
    for (int i = houseInd + 1; i <= numSeeds; i++) {
        game.grid[i % (NUM_HOUSES * NUM_PLAYERS)]++;
    }

    i = (i - 1) % (NUM_HOUSES * NUM_PLAYERS)
    while ((game.grid[i] == 2 || game.grid[i] == 3) & (i % NUM_HOUSES != move.numPlayer)) {
        game.scores[move.numPlayer] += c;
        game.grid[i] = 0;
    }
}