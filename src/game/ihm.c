#include "ihm.h"
#include <stdio.h>
#include <string.h>


int maxCellWidth(const Game *game, int gridY, int gridX) {
    int max = 0;
    for (int i = 0; i < gridY; ++i)
        for (int j = 0; j < gridX; ++j)
            if (game->grid[i][j] > max) {
                max = game->grid[i][j];
            }

    int w = 1;
    while (max >= 10) {
        max /= 10;
        w++;
    }

    return w;
}

void printGridMessage (char message[], Game *game, int gridX, int gridY, char usernames[][BUF_SIZE]) {
    int cellW = maxCellWidth(game, gridY, gridX);

    message += sprintf(message, "==========\n\n");
    message += sprintf(message, "Grille (%dx%d) rotation=%d\n\n", gridX, gridY, game->rotation);

    for (int i = 0; i < gridY; i++) {
        message += sprintf(message, "        |");
        if ((game->rotation == 0 && i == 0) || (game->rotation == 1 && i > 0)) {
            for (int j = gridX - 1; j >= 0; j--) {
                message += sprintf(message, " %*d |", cellW, game->grid[i][j]);
            }
        } else if ((game->rotation == 0 && i > 0) || (game->rotation == 1 && i == 0)) {
            for (int j = 0; j < gridX; j++) {
                message += sprintf(message, " %*d |", cellW, game->grid[i][j]);
            }
        }

        message += sprintf(message, "   %s : %d seeds\n", usernames[i], game->scores[i]);
    }

    message += sprintf(message, "\ncase n° |");
    for (int j = 0; j < gridX; j++) {
        message += sprintf(message, " %*d |", cellW, j);
    }

    message += sprintf(message, "\n\n==========\n");
}

void printGameEndMessage(char message[], Game *game, int nbPlayers, int winner, char usernames[][BUF_SIZE]) {
    char toAdd[BUF_SIZE] = "\0";

    strcat(message, "=== GAME OVER ===\nFinal Scores:\n");

    for (int i = 0; i < nbPlayers; i++) {
        sprintf(toAdd, "    %s: %d\n", usernames[i], game->scores[i]);
        strcat(message, toAdd);
    }

    if (winner >= 0) {
        sprintf(toAdd, "%s wins!\n", usernames[winner]);
    } else {
        sprintf(toAdd, "It’s a draw!\n");
    }
    strcat(message, toAdd);
}