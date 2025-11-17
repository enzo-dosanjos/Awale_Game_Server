#include <string.h>

#include "clientManager.h"


void initClient(Client *clients, int *actualClient, SOCKET sock,
                char username[], char password[])
// Initializes a new client with the provided socket, username, and password.
// Errors : none
{
    clients[*actualClient].sock = sock;
    strncpy(clients[*actualClient].username, username, BUF_SIZE - 1);
    strncpy(clients[*actualClient].password, password, BUF_SIZE - 1);
    clients[*actualClient].gameId = NULL;
    clients[*actualClient].numFriends = 0;
    clients[*actualClient].numPendingChallengesTo = 0;
    clients[*actualClient].numPendingChallengesFrom = 0;
    clients[*actualClient].bio[0] = '\0';
    clients[*actualClient].private = 0;
    clients[*actualClient].stats.gamesPlayed = 0;
    clients[*actualClient].stats.gamesWon = 0;
    clients[*actualClient].stats.gamesLost = 0;
    clients[*actualClient].stats.gamesDrawn = 0;
    clients[*actualClient].stats.averageMovesToWin = 0.0;
    clients[*actualClient].stats.totalSeedsCollected = 0;
}

int addChallenge(Client *challenger, Client *challenged)
// Adds a challenge from the challenger to the challenged client.
// Errors : already challenged, max pending challenges reached
{
   // check if already challenged
   for (int i = 0; i < challenger->numPendingChallengesTo; i++)
   {
      if (strcmp(challenger->pendingChallengesTo[i], challenged->username) == 0)
      {
         return 0;
      }
   }

   // max pending challenges check
   if (challenger->numPendingChallengesTo >= MAX_PENDING_CHALLENGES)
   {
      return -1;
   }

   // max pending challenges check for challenged
   if (challenger->numPendingChallengesFrom >= MAX_PENDING_CHALLENGES)
   {
      return -2;
   }

   strcpy(challenger->pendingChallengesTo[challenger->numPendingChallengesTo], challenged->username);
   challenger->numPendingChallengesTo++;

   strcpy(challenged->pendingChallengesFrom[challenged->numPendingChallengesFrom], challenger->username);
   challenged->numPendingChallengesFrom++;

   return 1;
}

int removeChallenge(Client *client, Client *challenged)
// Removes a challenge from the client to the challenged client.
// Errors : no pending challenge from that user
{
   int found = 0;
   for (int i = 0; i < client->numPendingChallengesTo; i++)
   {
      if (strcmp(client->pendingChallengesTo[i], challenged->username) == 0)
      {
         // Shift remaining challenges down
         for (int j = i; j < client->numPendingChallengesTo - 1; j++)
         {
            strcpy(client->pendingChallengesTo[j], client->pendingChallengesTo[j + 1]);
         }
         client->numPendingChallengesTo--;
         found = 1;
         break;
      }
   }

   if (!found)
   {
      return 0;
   }

   for (int i = 0; i < challenged->numPendingChallengesFrom; i++)
   {
      if (strcmp(challenged->pendingChallengesFrom[i], client->username) == 0)
      {
         // Shift remaining challenges down
         for (int j = i; j < challenged->numPendingChallengesFrom - 1; j++)
         {
            strcpy(challenged->pendingChallengesFrom[j], challenged->pendingChallengesFrom[j + 1]);
         }
         challenged->numPendingChallengesFrom--;
         break;
      }
   }

   return 1;
}

void clearSentChallenge(Client *client)
// Clears all challenges sent by the client.
// Errors : none
{
   client->numPendingChallengesTo = 0;
}

void clearReceivedChallenge(Client *client)
// Clears all challenges received by the client.
// Errors : none
{
   client->numPendingChallengesFrom = 0;
}

Client *findConnectedClientByUsername(Client **connectedClients,
                                      int actualConnected, char username[])
// Finds a connected client by their username.
// Returns a pointer to the Client if found, otherwise NULL.
// Errors : not found
{
    for (int i = 0; i < actualConnected; i++)
    {
        if (strcmp(connectedClients[i]->username, username) == 0)
        {
            return connectedClients[i];
        }
    }
    return NULL; // Not found
}

Client *findClientByUsername(Client *clients, int actualClient, char username[])
// Finds a client by their username.
// Returns a pointer to the Client if found, otherwise NULL.
// Errors : not found
{
    for (int i = 0; i < actualClient; i++)
    {
        if (strcmp(clients[i].username, username) == 0)
        {
            return &clients[i];
        }
    }
    return NULL; // Not found
}


int findClientIndex(Client **connectedClients, int actualConnected,
                    Client *client)
// Finds the index of a connected client.
// Returns the index if found, otherwise -1.
// Errors : not found
{
    for (int i = 0; i < actualConnected; i++)
    {
        if (connectedClients[i] == client)
        {
            return i;
        }
    }
    return -1; // Not found
}