#ifndef AWALE_GAME_SERVER_COMMANDS_H
#define AWALE_GAME_SERVER_COMMANDS_H

#include "gameLogic.h"
#include "gameServer.h"

int challenge(Client *clients, Client challenger, int actual, char username[]);

int acceptChallenge(Client *clients, Client *client, int actual, char challenger[], GameSession *gameSession);

void listClients(Client *clients, int actual, Client requester);

void listGames(GameSession *gameSessions, int actualGame, Client requester);

#endif //AWALE_GAME_SERVER_COMMANDS_H