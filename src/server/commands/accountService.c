#include <string.h>
#include <stdio.h>

#include "accountService.h"




int account_signUp(Client *clients, int *actualClient,
                   Client **connectedClients, int *actualConnected,
                   SOCKET *lobby, int *actualLobby, int lobbyIndex,
                   char *username, char *password)
// Registers a new user if the username is not already taken and there is room for more users.
// Errors : too many players, username already in use
{
    if (*actualClient >= MAX_CLIENTS - 1)
    {
        char msg[] = "Error: Too many players.\n";
        writeClient(lobby[lobbyIndex], msg);
        return 0;
    }

    for (int i = 0; i < *actualClient; i++)
    {
        if (strcmp(clients[i].username, username) == 0)
        {
            char msg[] = "Error: Username is already in use.\n";
            writeClient(lobby[lobbyIndex], msg);
            return 0;
        }
    }

    initClient(clients, actualClient, lobby[lobbyIndex], username, password);

    connectedClients[*actualConnected] = &clients[*actualClient];
    (*actualClient)++;
    (*actualConnected)++;

    char msg[BUF_SIZE] = "\0";
    sprintf(msg, "Welcome, %s! You can now challenge your first opponent!\n", username);
    writeClient(lobby[lobbyIndex], msg);

    removeFromLobby(lobby, actualLobby, lobbyIndex);

    return 1;
}

int account_login(Client *clients, int *actualClient,
                  Client **connectedClients, int *actualConnected,
                  GameSession **activeGameSessions, int *numActiveGames,
                  SOCKET *lobby, int *actualLobby, int lobbyIndex,
                  char *username, char *password)
// Logs in an existing user if the username and password match, handling multiple connections and disconnections as needed.
// Errors : too many connections, username not found, wrong password
{
    if (*actualConnected >= MAX_CONNECTED_CLIENTS - 1)
    {
        char msg[] = "Error: Too many simultaneous connections. Please wait.\n";
        writeClient(lobby[lobbyIndex], msg);
        return 0;
    }

    int i = 0;
    int usernameFound = 0;
    int passwordOkay = 0;

    for (i = 0; i < *actualClient; i++)
    {
        if (strcmp(clients[i].username, username) == 0)
        {
            usernameFound = 1;
            if (strcmp(clients[i].password, password) == 0)
            {
                passwordOkay = 1;
            }
            break;
        }
    }

    if (!usernameFound)
    {
        char msg[] = "Error: Username not found.\n";
        writeClient(lobby[lobbyIndex], msg);
        return 0;
    }

    if (!passwordOkay)
    {
        char msg[] = "Error: Wrong password.\n";
        writeClient(lobby[lobbyIndex], msg);
        return 0;
    }

    if (clients[i].sock >= 0)
    {
        char msg[] = "You've been disconnected because of a connection on another device.\n";
        writeClient(clients[i].sock, msg);
        account_quit(connectedClients, actualConnected, &clients[lobbyIndex], activeGameSessions, numActiveGames);
    }

    clients[i].sock = lobby[lobbyIndex];

    connectedClients[*actualConnected] = &clients[i];
    (*actualConnected)++;

    char msg[BUF_SIZE] = "\0";
    sprintf(msg, "Welcome back, %s!\n", username);
    writeClient(lobby[lobbyIndex], msg);

    removeFromLobby(lobby, actualLobby, lobbyIndex);

    return 1;
}

int account_quit(Client **connectedClients, int *actualConnected,
                 Client *client,
                 GameSession **activeGameSessions, int *numActiveGames)
// Handles the disconnection of a client, including game termination if the client is in a game.
// Errors : none
{
    int allSaved = 0;

    // If the client is in a game, handle game termination
    if (client->gameId != NULL)
    {
        GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);

        if (gameSession != NULL)
        {
            if (gameSession->endGame == -1)
            {
                // Notify the opponent
                for (int i = 0; i < NUM_PLAYERS; i++)
                {
                    if (gameSession->players[i] != client)
                    {
                        char msg[2 * BUF_SIZE];
                        snprintf(msg, 2 * BUF_SIZE, "The opponent %s has disconnected. The game has been saved.\n", client->username);
                        writeClient(gameSession->players[i]->sock, msg);
                        gameSession->players[i]->gameId = NULL;
                    }
                }

                removeActiveGameSession(activeGameSessions, numActiveGames, gameSession->id);
            }
            else if (gameSession->endGame == NUM_PLAYERS)
            {
                allSaved = 1;

                for (int i = 0; i < NUM_PLAYERS; i++)
                {
                    
                    if (gameSession->players[i] == client)
                    {
                        gameSession->saveAnswered[i] = 1;
                    }
                    else if (!gameSession->saveAnswered[i])
                    {
                        allSaved = 0;
                    }
                }
            }
        }

        client->gameId = NULL;
    }

    closesocket(client->sock);
    client->sock = -1;

    char buffer[BUF_SIZE];
    strncpy(buffer, client->username, BUF_SIZE - 1);
    strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);

    int i = findClientIndex(connectedClients, *actualConnected, client);
    removeClient(connectedClients, actualConnected, i);

    sendMessageToAllClients(connectedClients, *actualConnected, client->username, buffer, 1);

    if (allSaved)
    {
        return -1;
    }
    return 1;
}
