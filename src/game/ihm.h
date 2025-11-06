#ifndef AWALE_GAME_SERVER_IHM_H
#define AWALE_GAME_SERVER_IHM_H

#include "gameUtils.h"

#define BUF_SIZE    1024

void printGridMessage(char message[], Game *game, int gridX, int gridY, char usernames[][BUF_SIZE]);

void printGameEndMessage(char message[], Game *game, int nbPlayers, int winner, char usernames[][BUF_SIZE]);

#endif //AWALE_GAME_SERVER_IHM_H