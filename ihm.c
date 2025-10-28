#include "ihm.h"
#include <stdio.h>

void printGrid (Game game, int gridX, int gridY) {
    for (int i = 0; i < gridY; i++) {
        if ((game.rotation == 0 && i == 0) || (game.rotation == 1 && i > 0)) {
            for (int j = gridX - 1; j >= 0; j--) {
                printf("%d ", game.grid[i][j]);
            }
        } else if ((game.rotation == 0 && i > 0) || (game.rotation == 1 && i == 0)) {
            for (int j = 0; j < gridX; j++) {
                printf("%d ", game.grid[i][j]);
            }
        }
        printf("\n");
    }
}

void printGameEnd(Game game, int numPlayers, int winner) {
    printf("\n=== GAME OVER ===\n");
    printf("Final Scores:\n");
    for (int i = 0; i < numPlayers; i++) {
        printf("Player %d: %d\n", i + 1, game.scores[i]);
    }

    if (winner >= 0) {
        printf("Player %d wins!\n", winner + 1);
    } else {
        printf("It’s a draw!\n");
    }
}