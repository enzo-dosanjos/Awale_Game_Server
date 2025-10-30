#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int challenge(Client *clients, Client *challenger, int actual, char username[]) {
    if (strcmp(challenger->username, username) == 0) {
        char msg[] = "Error: You cannot challenge yourself.";
        write_client(challenger->sock, msg);
        return 0;
    }

    Client *challenged = findClientByUsername(clients, actual, username);
    if (challenged == NULL) {
        char msg[] = "Error: User not found.";
        write_client(challenger->sock, msg);
        return 0;
    }

    if (add_challenge(challenger, challenged) == 0) {
        return 0;
    }

    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_FROM %s", challenger->username);
    sendMessageToClient(clients, NULL, actual, username, message);

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

    if (client->gameId != NULL) {
        char msg[] = "Error: You are already in a game.";
        write_client(client->sock, msg);
        return 0;
    }

    if (challengerClient->gameId != NULL) {
        char msg[] = "Error: Challenger is already in a game.";
        write_client(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    remove_challenge(challengerClient, client);

    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_ACCEPTED_BY %s", client->username);
    sendMessageToClient(clients, NULL, actual, challenger, message);

    strcpy(message, "Enter rotation (0 for counter-clockwise, 1 for clockwise): ");
    sendMessageToClient(clients, NULL, actual, client->username, message);

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

int declineChallenge(Client *clients, Client *client, int actual, char challenger[]) {
    Client *challengerClient = findClientByUsername(clients, actual, challenger);
    if (challengerClient == NULL) {
        // challenger not found
        char msg[] = "Error : challenger not found";
        write_client(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    remove_challenge(challengerClient, client);

    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_DECLINED_BY %s", client->username);
    sendMessageToClient(clients, NULL, actual, challenger, message);

    return 1;
}

void seePendingReq(Client *client) {
    char message[BUF_SIZE];
    message[0] = '\0';

    if (client->numPendingChallengesFrom == 0) {
        strncat(message, "No pending challenges.\n", BUF_SIZE - strlen(message) - 1);
    } else {
        strncat(message, "Pending challenges from:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < client->numPendingChallengesFrom; i++) {
            strncat(message, client->pendingChallengesFrom[i], BUF_SIZE - strlen(message) - 1);
            strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
        }
    }
    write_client(client->sock, message);
}

void seeSentReq(Client *client) {
    char message[BUF_SIZE];
    message[0] = '\0';

    if (client->numPendingChallengesTo == 0) {
        strncat(message, "No sent challenges.\n", BUF_SIZE - strlen(message) - 1);
    } else {
        strncat(message, "Sent challenges to:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < client->numPendingChallengesTo; i++) {
            strncat(message, client->pendingChallengesTo[i], BUF_SIZE - strlen(message) - 1);
            strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
        }
    }
    write_client(client->sock, message);
}

void clearPendingReq(Client *client) {
    clear_received_challenge(client);
    char msg[] = "All received pending challenges cleared.";
    write_client(client->sock, msg);
}

void clearSentReq(Client *client) {
    clear_sent_challenge(client);
    char msg[] = "All sent pending challenges cleared.";
    write_client(client->sock, msg);
}

int removeSentReq(Client *clients, Client *client, int actual, char username[]) {
    Client *challengedClient = findClientByUsername(clients, actual, username);
    if (challengedClient == NULL) {
        char msg[] = "Error: User not found.";
        write_client(client->sock, msg);
        return 0;
    }

    if (remove_challenge(client, challengedClient)) {
        char msg[] = "Pending challenge removed.";
        write_client(client->sock, msg);
    }

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

void sendMP(Client *clients, int actual, char *username, char *message) {
    Client *client = findClientByUsername(clients, actual, username);
    if (client != NULL) {
        write_client(client->sock, message);
    }
}
