#include "ihm.h"
#include <stdio.h>

void printGrid (Game game, int gridX, int gridY) {
    for (int i = 0; i < gridY; i++) {
        if (i > 0 || i > gridY - 2) {  // handle rotation
            for (int j = gridX - 1; j >= 0; j--) {
                printf("%d ", game.grid[i][j]);
            }
        } else {
            for (int j = 0; j < gridX; j++) {
                printf("%d ", game.grid[i][j]);
            }
        }
        printf("\n");
    }
}