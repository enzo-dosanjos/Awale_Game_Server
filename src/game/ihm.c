#include "ihm.h"
#include <stdio.h>
#include <string.h>

void printGridMessage (char message[], Game *game, int gridX, int gridY, char usernames[][BUF_SIZE]) {
    char toAdd[BUF_SIZE] = "\0";

    strcat(message, "==========\n");

    for (int i = 0; i < gridY; i++) {
        if ((game->rotation == 0 && i == 0) || (game->rotation == 1 && i > 0)) {
            for (int j = gridX - 1; j >= 0; j--) {
                sprintf(toAdd, "%d ", game->grid[i][j]);
                strcat(message, toAdd);
            }
        } else if ((game->rotation == 0 && i > 0) || (game->rotation == 1 && i == 0)) {
            for (int j = 0; j < gridX; j++) {
                sprintf(toAdd, "%d ", game->grid[i][j]);
                strcat(message, toAdd);
            }
        }

        sprintf(toAdd, "%s : %d seeds\n", usernames[i], game->scores[i]);
        strcat(message, toAdd);
    }

    strcat(message, "==========\n");
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