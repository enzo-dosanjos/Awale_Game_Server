#ifndef AWALE_GAME_SERVER_COMMANDS_H
#define AWALE_GAME_SERVER_COMMANDS_H

#include "gameLogic.h"
#include "gameServer.h"

void challenge(Client *clients, Client challenger, int actual, char username[]);

GameSession acceptChallenge(Client *clients, Client challenger, int actual, char username[]);

#endif //AWALE_GAME_SERVER_COMMANDS_H