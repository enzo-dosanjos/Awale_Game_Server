#ifndef AWALE_GAME_SERVER_IHM_H
#define AWALE_GAME_SERVER_IHM_H

#include "gameUtils.h"
#include "../constants.h"


int maxCellWidth(const Game *game, int gridY, int gridX);

void printGridMessage(char message[], Game *game, int gridX, int gridY, char usernames[][BUF_SIZE]);

void printGameEndMessage(char message[], Game *game, int nbPlayers, int winner, char usernames[][BUF_SIZE]);

#endif //AWALE_GAME_SERVER_IHM_H