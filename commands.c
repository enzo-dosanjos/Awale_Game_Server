#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int challenge(Client *clients, Client challenger, int actual, char username[]) {
    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_FROM %s", challenger.username);
    sendMessageToClient(clients, actual, username, message);

    return 1;
}

int acceptChallenge(Client *clients, Client *client, int actual, char challenger[], GameSession *gameSession) {
    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_ACCEPTED_BY %s", client->username);
    sendMessageToClient(clients, actual, challenger, message);

    strcpy(message, "Enter rotation (0 for counter-clockwise, 1 for clockwise): ");
    sendMessageToClient(clients, actual, client->username, message);

    char rotationStr[2];
    read_client(client->sock, rotationStr);
    int rotation = atoi(rotationStr);

    Game game = startGame(rotation);

    gameSession->game = game;
    gameSession->currentPlayer = 0;

    Client *challengerClient = findClientByUsername(clients, actual, challenger);
    if (challengerClient == NULL) {
        // challenger not found
        char msg[] = "Error : challenger not found";
        write_client(client->sock, msg);
        return 0;
    }
    gameSession->players[0] = *challengerClient;
    gameSession->players[1] = *client;
    gameSession->id = (int) time(NULL);  // timestamp

    challengerClient->gameId = &gameSession->id;
    client->gameId = &gameSession->id;

    return 1;
}

void listClients(Client *clients, int actual, Client requester) {
    char message[BUF_SIZE];
    message[0] = 0;
    strncat(message, "Connected users:\n", BUF_SIZE - strlen(message) - 1);

    // Find the maximum username length for formatting
    int maxLen = 0;
    for (int j = 0; j < actual; j++) {
        int currLen = (int) strlen(clients[j].username);
        if (currLen > maxLen) {
            maxLen = currLen;
        }
    }

    for (int i = 0; i < actual; i++) {
        strncat(message, clients[i].username, BUF_SIZE - strlen(message) - 1);

        int pad = maxLen - (int)strlen(clients[i].username) + 1;
        if (pad < 1) {
            pad = 1;
        }

        char spaces[BUF_SIZE];
        memset(spaces, ' ', (size_t)pad);
        spaces[pad] = '\0';
        strncat(message, spaces, BUF_SIZE - strlen(message) - 1);

        if (clients[i].gameId != NULL) {
            strncat(message, "in game", BUF_SIZE - strlen(message) - 1);
        }

        strncat(message, "\n",   BUF_SIZE - strlen(message) - 1);
    }
    write_client(requester.sock, message);
}
