#ifndef AWALE_GAME_SERVER_GAMELOGIC_H
#define AWALE_GAME_SERVER_GAMELOGIC_H

#include "gameUtils.h"

Game startGame(int rotation, int numPlayers, int numHouses, int numSeeds);

int playMove(Game *game, Move move);

int nextPlayer(int currentPlayer, Game *game);

int endGame(Game *game);

int playerSelector(Game *game);

#endif // AWALE_GAME_SERVER_GAMELOGIC_H