#ifndef AWALE_GAME_SERVER_GAMELOGIC_H
#define AWALE_GAME_SERVER_GAMELOGIC_H

#include "../constants.h"
#include "gameUtils.h"

Game startGame(int rotation);

int playMove(Game *game, Move move);

int nextPlayer(int currentPlayer);

int endGame(Game *game);

int playerSelector();

#endif // AWALE_GAME_SERVER_GAMELOGIC_H