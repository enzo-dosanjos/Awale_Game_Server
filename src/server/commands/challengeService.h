#ifndef AWALE_GAME_SERVER_CHALLENGESERVICE_H
#define AWALE_GAME_SERVER_CHALLENGESERVICE_H


#include "../networking/serverUtils.h"
#include "../dataManagers/clientManager.h"
#include "../dataManagers/gameSessionManager.h"


int challenge_send(Client **connectedClients, Client *challenger,
                   int actualConnected, char username[]);

int challenge_accept(Client **connectedClients, Client *client,
                     int actualConnected, char challenger[],
                     int *numGames,
                     GameSession **activeGameSessions, int *numActiveGames);

int challenge_decline(Client *clients, Client *client,
                      int actualClients, char challenger[]);

int challenge_removeSent(Client *clients, Client *client,
                         int actualClient, char username[]);

void challenge_seePending(Client *client);

void challenge_seeSent(Client *client);

void challenge_clearPending(Client *client);

void challenge_clearSent(Client *client);

#endif //AWALE_GAME_SERVER_CHALLENGESERVICE_H
