#include "ihm.h"
#include "constants.h"
#include <stdlib.h>

Game startGame (int rotation);

int play (Game game, Move move);

int nextPlayer (int currentPlayer);

int endGame (Game game);