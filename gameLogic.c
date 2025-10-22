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

    // Ajouter le saut de la douzième case

    return 1;
}

int endGame (Game game) {
    // Collect remaining seeds
    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < NUM_HOUSES; j++) {
            game.scores[i] += game.grid[i][j];
            game.grid[i][j] = 0;
        }
    }

    // Determine winner
    int maxScore = -1;
    int winner = -1;
    int draw = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("%d\n", game.scores[i]);
        if (game.scores[i] > maxScore) {
            maxScore = game.scores[i];
            winner = i;
            draw = 0;
        } else if (game.scores[i] == maxScore) {
            draw = 1;
        }
    }

    return draw ? -1 : winner;
}

int main() {
    int rotation = 1;  // 0 for clockwise, 1 for counter-clockwise
    Game game = startGame(rotation);

    // while (not isGameOver(game)) {

    Move move;
    move.numPlayer = 1;
    move.houseNum = 5;

    // dans la future fonction play
    if (game.rotation == 0) {
        move.houseNum = (NUM_HOUSES - 1) - move.houseNum;
    }

    play(game, move);

    printGrid(game, NUM_HOUSES, NUM_PLAYERS);

    int winner = endGame(game);
    printGameEnd(game, NUM_PLAYERS, winner);

    freeGame(game);

    return 0;
}
