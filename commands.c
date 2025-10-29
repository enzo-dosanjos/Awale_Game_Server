#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int challenge(Client *clients, Client *challenger, int actual, char username[]) {
    if (challenger->numPendingChallenges >= MAX_PENDING_CHALLENGES) {
        char msg[] = "Error: You have reached the maximum number of pending challenges. Please wait for one to be accepted.";
        write_client(challenger->sock, msg);
        return 0;
    }

    if (strcmp(challenger->username, username) == 0) {
        char msg[] = "Error: You cannot challenge yourself.";
        write_client(challenger->sock, msg);
        return 0;
    }

    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_FROM %s", challenger->username);
    sendMessageToClient(clients, actual, username, message);

    strcpy(challenger->pendingChallenges[challenger->numPendingChallenges], username);
    challenger->numPendingChallenges++;

    return 1;
}

int acceptChallenge(Client *clients, Client *client, int actual, char challenger[], GameSession *gameSession) {
    Client *challengerClient = findClientByUsername(clients, actual, challenger);
    if (challengerClient == NULL) {
        // challenger not found
        char msg[] = "Error : challenger not found";
        write_client(client->sock, msg);
        return 0;
    }

    // Check if there is a pending challenge from the challenger
    int found = 0;
    for (int i = 0; i < challengerClient->numPendingChallenges; i++) {
        if (strcmp(challengerClient->pendingChallenges[i], client->username) == 0) {
            found = 1;
            // Remove all pending challenges from the challenger and the client
            challengerClient->numPendingChallenges = 0;
            client->numPendingChallenges = 0;
            break;
        }
    }

    if (!found) {
        char msg[] = "Error: No pending challenge from that user.";
        write_client(client->sock, msg);
        return 0;
    }

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

    gameSession->players[0] = *challengerClient;
    gameSession->players[1] = *client;
    gameSession->id = (int) time(NULL);  // timestamp

    challengerClient->gameId = &gameSession->id;
    client->gameId = &gameSession->id;

    return 1;
}

void listClients(Client *clients, int actual, Client requester) {
    char message[BUF_SIZE];
    message[0] = '\0';
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

void listGames(GameSession *gameSessions, int actualGame, Client requester) {
    char message[BUF_SIZE];
    message[0] = '\0';

    if (actualGame == 0) {
        strncat(message, "No ongoing games.\n", BUF_SIZE - strlen(message) - 1);
    } else {
        strncat(message, "Ongoing games:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < actualGame; i++) {
            char gameInfo[BUF_SIZE];
            snprintf(gameInfo, BUF_SIZE, "Game ID: %d | Players: %s vs %s\n",
                     gameSessions[i].id,
                     gameSessions[i].players[0].username,
                     gameSessions[i].players[1].username);
            strncat(message, gameInfo, BUF_SIZE - strlen(message) - 1);
        }
    }
    write_client(requester.sock, message);
}