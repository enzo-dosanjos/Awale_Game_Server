#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "serverUtils.h"

int initConnectionServer(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = {0};

   if (sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

void endConnection(int sock)
{
   closesocket(sock);
}

void clearClients(Client **connectedClients, int actualConnected)
{
   int i = 0;
   for (i = 0; i < actualConnected; i++)
   {
      closesocket(connectedClients[i]->sock);
   }
}

void removeClient(Client **connectedClients, int to_remove, int *actualConnected)
{
   /* we remove the client in the array */
   memmove(connectedClients + to_remove, connectedClients + to_remove + 1, (*actualConnected - to_remove - 1) * sizeof(Client *));
   /* number client - 1 */
   (*actualConnected)--;
}

void removeFromLobby(SOCKET *lobby, int to_remove, int *actualLobby)
{
   memmove(lobby + to_remove, lobby + to_remove + 1, (*lobby - to_remove - 1) * sizeof(SOCKET));
   (*actualLobby)--;
}

void sendMessageToAllClients(Client **connectedClients, char sender[], int actualConnected, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;

   if (from_server == 0)
   {
      strncpy(message, sender, BUF_SIZE - 1);
      strncat(message, " : ", sizeof message - strlen(message) - 1);
   }
   strncat(message, buffer, sizeof message - strlen(message) - 1);

   for (i = 0; i < actualConnected; i++)
   {
      if (connectedClients[i] != NULL)
      {
         writeClient(connectedClients[i]->sock, message);
      }
   }
}

void sendMessageToLobby(SOCKET *lobby, int actualLobby, const char *message)
{
   for (int i = 0; i < actualLobby; i++)
   {
      writeClient(lobby[i], message);
   }
}

void initClient(Client *clients, int *actualClient, SOCKET sock, char username[], char password[])
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

GameSession *initGameSession(GameSession *gameSessions, int *numGames, Game *game, int firstPlayer, Client *player1, Client *player2)
{
   GameSession *gameSession = &gameSessions[*numGames];

   gameSession->game = *game;
   gameSession->currentPlayer = firstPlayer;

   gameSession->numMoves = 1;

   gameSession->players[0] = player1;
   gameSession->players[1] = player2;
   gameSession->id = (int)time(NULL); // timestamp
   gameSession->endGameSuggested = -1;
   gameSession->saveAnswered = 0;
   gameSession->numViewers = 0;

   gameSession->numMovesRecorded = 0;
   gameSession->numGameMessages = 0;

   (*numGames)++;

   return gameSession;
}

void recordMove(GameSession *gameSession, const Move *move, const char *grid)
{
    if (gameSession->numMovesRecorded < MAX_MOVES_HISTORY)
    {
        MoveRecord *moveRecord = &gameSession->movesHistory[gameSession->numMovesRecorded];
        moveRecord->number = gameSession->numMoves;
        if (move == NULL) {
            moveRecord->playerNum = -1; // Indicate no move was made
            moveRecord->house = -1;
        } else {
            moveRecord->playerNum = move->numPlayer;
            moveRecord->house = move->houseNum;
        }
        moveRecord->t = time(NULL);

        strcpy(moveRecord->grid, grid);
        moveRecord->grid[BUF_SIZE - 1] = '\0';  // Ensure null-termination

        gameSession->numMovesRecorded++;
    }
}

void recordChat(GameSession *gameSession, const char *sender, const char *text)
{
    if (gameSession->numGameMessages < MAX_MESSAGES_HISTORY)
    {
        ChatRecord *chatRecord = &gameSession->gameMessages[gameSession->numGameMessages];
        strcpy(chatRecord->sender, sender);
        chatRecord->sender[BUF_SIZE - 1] = '\0';  // Ensure null-termination

        strcpy(chatRecord->text, text);
        chatRecord->text[BUF_SIZE - 1] = '\0';

        chatRecord->t = time(NULL);

        gameSession->numGameMessages++;
    }
}

void formatTime(time_t t, char *out, size_t outSize) {
    struct tm *lt = localtime(&t);
    if (lt)
    {
        strftime(out, outSize, "%Y-%m-%d %H:%M:%S", lt);
    }
    else
    {
        strncpy(out, "unknown", outSize - 1);
        out[outSize - 1] = '\0';
    }
}

Client *findConnectedClientByUsername(Client **connectedClients, int actualConnected, char username[])
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


int findClientIndex(Client **connectedClients, int actualConnected, Client *client)
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

GameSession *findGameSessionByClient(Client *client, GameSession **gameSessions, int actualGame)
{
   int i = 0;
   while ((i < actualGame) && (gameSessions[i]->id != *(client->gameId)))
      i++;
   if (i == actualGame)
   {
      return NULL; // Not found
   }

   return gameSessions[i];
}

void sendMessageToClient(Client **connectedClients, Client *sender, int actualConnected, char username[], const char *buffer)
{
   char message[BUF_SIZE];
   message[0] = '\0';
   if (sender != NULL)
   {
      strncpy(message, sender->username, BUF_SIZE - 1);
      strncat(message, " : ", sizeof message - strlen(message) - 1);
   }

   Client *client = findConnectedClientByUsername(connectedClients, actualConnected, username);
   if (client != NULL)
   {
      strncat(message, buffer, sizeof message - strlen(message) - 1);
      writeClient(client->sock, message);
   }
}

int readClient(SOCKET sock, char *buffer)
{
   int n = 0;

   if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

void writeClient(SOCKET sock, const char *buffer)
{
   if (send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int addChallenge(Client *challenger, Client *challenged)
{
   // check if already challenged
   for (int i = 0; i < challenger->numPendingChallengesTo; i++)
   {
      if (strcmp(challenger->pendingChallengesTo[i], challenged->username) == 0)
      {
         char msg[] = "Error: You have already challenged this user.";
         writeClient(challenger->sock, msg);
         return 0;
      }
   }

   // max pending challenges check
   if (challenger->numPendingChallengesTo >= MAX_PENDING_CHALLENGES)
   {
      char msg[] = "Error: You have reached the maximum number of pending challenges. Please wait for one to be accepted.";
      writeClient(challenger->sock, msg);
      return 0;
   }

   // max pending challenges check for challenged
   if (challenger->numPendingChallengesFrom >= MAX_PENDING_CHALLENGES)
   {
      char msg[] = "Error: The user you are trying to challenge has reached the maximum number of pending challenges. Please try again later.";
      writeClient(challenged->sock, msg);
      return 0;
   }

   strcpy(challenger->pendingChallengesTo[challenger->numPendingChallengesTo], challenged->username);
   challenger->numPendingChallengesTo++;

   strcpy(challenged->pendingChallengesFrom[challenged->numPendingChallengesFrom], challenger->username);
   challenged->numPendingChallengesFrom++;

   return 1;
}

int removeChallenge(Client *client, Client *challenged)
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
      char msg[] = "Error: No pending challenge from that user.";
      writeClient(challenged->sock, msg);
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
{
   client->numPendingChallengesTo = 0;
}

void clearReceivedChallenge(Client *client)
{
   client->numPendingChallengesTo = 0;
}

GameSession *findGameSessionByViewer(GameSession **gameSessions, int actualGame, Client *viewer)
{
   for (int i = 0; i < actualGame; i++)
   {
      for (int j = 0; j < gameSessions[i]->numViewers; j++)
      {
         if (gameSessions[i]->viewers[j] == viewer)
         {
            return gameSessions[i];
         }
      }

      for (int k = 0; k < NUM_PLAYERS; k++)
      {
         if (gameSessions[i]->players[k] == viewer)
         {
            return gameSessions[i];
         }
      }
   }
   return NULL; // Not found
}

int removeActiveGameSession(GameSession **activeGameSessions, int *numGames, int gameId)
{
   int foundIndex = -1;
   for (int i = 0; i < *numGames; i++)
   {
      if (activeGameSessions[i]->id == gameId)
      {
         foundIndex = i;
         break;
      }
   }

   if (foundIndex == -1)
   {
      return 0; // Not found
   }

   // Shift remaining game sessions down
   for (int i = foundIndex; i < *numGames - 1; i++)
   {
      activeGameSessions[i] = activeGameSessions[i + 1];
   }
   (*numGames)--;

   return 1;
}

int removeGameSession(GameSession *gameSessions, int *numGames, int gameId)
{
   int foundIndex = -1;
   for (int i = 0; i < *numGames; i++)
   {
      if (gameSessions[i].id == gameId)
      {
         foundIndex = i;
         break;
      }
   }

   if (foundIndex == -1)
   {
      return 0; // Not found
   }

   // Shift remaining game sessions down
   for (int i = foundIndex; i < *numGames - 1; i++)
   {
      gameSessions[i] = gameSessions[i + 1];
   }
   (*numGames)--;

   return 1;
}

void freeServerData(Client **connectedClients, int actualConnected, SOCKET *lobby, int actualLobby, GameSession **gameSessions, int numGames)
{
    int i;
    // Close all client sockets
    clearClients(connectedClients, actualConnected);

    for (i = 0; i < actualLobby; i++)
    {
        closesocket(lobby[i]);
    }

    // Free all dynamically allocated game data
    for (i = 0; i < numGames; i++)
    {
        freeGame(&gameSessions[i]->game);
    }
}

