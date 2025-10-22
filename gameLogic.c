#include "gameLogic.h"
#include <stdlib.h>


int**  initGrid () {
    int** grid = malloc(NUM_PLAYERS * sizeof(int*));
    int* data = malloc(NUM_PLAYERS * NUM_HOUSES * sizeof(int));

    for (int i = 0; i < NUM_PLAYERS; i++) {
        grid[i] = data + i * NUM_HOUSES;
    }

    int numSeedsPerHouse = NUM_SEEDS / (NUM_HOUSES * NUM_PLAYERS);
    
    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < NUM_HOUSES; j++) {
            grid[i][j] = numSeedsPerHouse;
        }
    }

    return grid;
}

Game startGame (int rotation) {
    Game game;
    game.grid = initGrid();
    game.rotation = rotation;
    game.scores = malloc(NUM_PLAYERS * sizeof(int));
    for (int i = 0; i < NUM_PLAYERS; i++) {
        game.scores[i] = 0;
    }

    return game;

}

int play (Game game, Move move) {
    int *gameGrid = &game.grid[0][0];

    int houseInd = move.numPlayer * NUM_HOUSES + move.houseNum;
    int numSeeds = game.grid[move.numPlayer][move.houseNum];

    gameGrid[houseInd] = 0;

    int i;
    for (i = houseInd + 1; i <= houseInd + 1 + numSeeds; i++) {
        gameGrid[i % (NUM_HOUSES * NUM_PLAYERS)]++;
    }

    i = (i - 1) % (NUM_HOUSES * NUM_PLAYERS);

    while ((gameGrid[i] == 2 || gameGrid[i] == 3) & (i % NUM_HOUSES != move.numPlayer)) {
        game.scores[move.numPlayer] += gameGrid[i];
        gameGrid[i] = 0;
        i--;
    }

    return 1;
}

int main() {
    int rotation = 1;  // 0 or 1
    Game game = startGame(rotation);

    Move move;
    move.numPlayer = 1;
    move.houseNum = 5;

    // dans la future fonction play
    if (game.rotation == 0) {
        move.houseNum = (NUM_HOUSES - 1) - move.houseNum;
    }

    play(game, move);

    printGrid(game, NUM_HOUSES, NUM_PLAYERS);

    return 0;
}
