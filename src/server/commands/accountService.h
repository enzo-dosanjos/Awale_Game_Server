#ifndef AWALE_GAME_SERVER_LOBBYSERVICE_H
#define AWALE_GAME_SERVER_LOBBYSERVICE_H


#include "../networking/serverUtils.h"
#include "../dataManagers/clientManager.h"
#include "../dataManagers/gameSessionManager.h"


int account_signUp(Client *clients, int *actualClient,
                   Client **connectedClients, int *actualConnected,
                   SOCKET *lobby, int *actualLobby, int lobbyIndex,
                   char *username, char *password);

int account_login(Client *clients, int *actualClient,
                  Client **connectedClients, int *actualConnected,
                  GameSession **activeGameSessions, int *numActiveGames,
                  SOCKET *lobby, int *actualLobby, int lobbyIndex,
                  char *username, char *password);

int account_quit(Client **connectedClients, int *actualConnected,
                 Client *client,
                 GameSession **activeGameSessions, int *numActiveGames);

#endif //AWALE_GAME_SERVER_LOBBYSERVICE_H
