#ifndef AWALE_GAME_SERVER_GENERALSERVICE_H
#define AWALE_GAME_SERVER_GENERALSERVICE_H

#include "../networking/serverUtils.h"
#include "../dataManagers/clientManager.h"
#include "../dataManagers/gameSessionManager.h"


void general_listClients(Client **connectedClients,
                         int actualConnected, Client requester);

void general_listGames(GameSession **gameSessions,
                       int actualGame, Client requester);

void general_sendHelp(SOCKET sock, int loggedIn);

#endif //AWALE_GAME_SERVER_GENERALSERVICE_H
