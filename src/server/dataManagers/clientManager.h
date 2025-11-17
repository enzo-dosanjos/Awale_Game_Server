#ifndef AWALE_GAME_SERVER_CLIENTMANAGER_C_H
#define AWALE_GAME_SERVER_CLIENTMANAGER_C_H

#include "../data/data.h"


void initClient(Client *clients, int *actualClient, SOCKET sock,
                char username[], char password[]);

// Challenges
int addChallenge(Client *challenger, Client *challenged);

int removeChallenge(Client *client, Client *challenged);

void clearSentChallenge(Client *client);

void clearReceivedChallenge(Client *client);

// Lookups
Client *findConnectedClientByUsername(Client **connectedClients,
                                      int actualConnected,
                                      char username[]);

Client *findClientByUsername(Client *clients, int actualClient,
                             char username[]);

int findClientIndex(Client **connectedClients, int actualConnected,
                    Client *client);

#endif //AWALE_GAME_SERVER_CLIENTMANAGER_C_H
