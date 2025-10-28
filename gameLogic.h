#ifndef AWALE_GAME_SERVER_GAMELOGIC_H
#define AWALE_GAME_SERVER_GAMELOGIC_H

#include "ihm.h"
#include "constants.h"
#include <stdlib.h>

Game startGame (int rotation);

int playMove (Game game, Move move);

int nextPlayer (int currentPlayer);

int endGame (Game game);

int playGame();

#endif //AWALE_GAME_SERVER_GAMELOGIC_H