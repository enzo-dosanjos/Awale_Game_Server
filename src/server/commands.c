#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int challenge(Client *clients, Client *challenger, int actual, char username[]) {
    if (strcmp(challenger->username, username) == 0) {
        char msg[] = "Error: You cannot challenge yourself.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    Client *challenged = findClientByUsername(clients, actual, username);
    if (challenged == NULL) {
        char msg[] = "Error: User not found.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    if (addChallenge(challenger, challenged) == 0) {
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
        char msg[] = "Error : challenger not found\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (client->gameId != NULL) {
        char msg[] = "Error: You are already in a game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (challengerClient->gameId != NULL) {
        char msg[] = "Error: Challenger is already in a game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    if (removeChallenge(challengerClient, client) == 0) {
        return 0;
    }

    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_ACCEPTED_BY %s", client->username);
    sendMessageToClient(clients, NULL, actual, challenger, message);

    strcpy(message, "Enter rotation (0 for counter-clockwise, 1 for clockwise): \n");
    sendMessageToClient(clients, NULL, actual, client->username, message);

    char rotationStr[2];
    readClient(client->sock, rotationStr);
    int rotation = atoi(rotationStr);

    Game game = startGame(rotation);

    gameSession->game = game;
    gameSession->currentPlayer = playerSelector();

    gameSession->players[0] = challengerClient;
    gameSession->players[1] = client;
    gameSession->id = (int) time(NULL);  // timestamp
    gameSession->endGameSuggested = -1;

    challengerClient->gameId = &gameSession->id;
    client->gameId = &gameSession->id;

    message[0] = '\0';
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        strcpy(usernames[i], gameSession->players[i]->username);
    }
    printGridMessage(message, &gameSession->game, NUM_HOUSES, NUM_PLAYERS, usernames);
    writeClient(client->sock, message);
    writeClient(challengerClient->sock, message);
    writeClient(gameSession->players[gameSession->currentPlayer]->sock, "It's your turn to shine!\n");

    return 1;
}

int declineChallenge(Client *clients, Client *client, int actual, char challenger[]) {
    Client *challengerClient = findClientByUsername(clients, actual, challenger);
    if (challengerClient == NULL) {
        // challenger not found
        char msg[] = "Error : challenger not found\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    removeChallenge(challengerClient, client);

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
    writeClient(client->sock, message);
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
    writeClient(client->sock, message);
}

void clearPendingReq(Client *client) {
    clearReceivedChallenge(client);
    char msg[] = "All received pending challenges cleared.\n";
    writeClient(client->sock, msg);
}

void clearSentReq(Client *client) {
    clearSentChallenge(client);
    char msg[] = "All sent pending challenges cleared.\n";
    writeClient(client->sock, msg);
}

int removeSentReq(Client *clients, Client *client, int actual, char username[]) {
    Client *challengedClient = findClientByUsername(clients, actual, username);
    if (challengedClient == NULL) {
        char msg[] = "Error: User not found.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (removeChallenge(client, challengedClient)) {
        char msg[] = "Pending challenge removed.\n";
        writeClient(client->sock, msg);
    }

    return 1;
}

int move(Client *client, GameSession *gameSessions, int actualGame, int house) {
    if (!client->gameId) {
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, gameSessions, actualGame);
    if (gameSession == NULL) {
        return 0;
    }

    Move move;
    move.houseNum = house;

    if (client != gameSession->players[gameSession->currentPlayer]) {
        char msg[] = "Error: It's not your turn, please wait for the opponent to make their move.\n";
        writeClient(client->sock, msg);
        return 0;
    }
    
    int next = nextPlayer(gameSession->currentPlayer);
    Client *opponent = gameSession->players[next];
    move.numPlayer = gameSession->currentPlayer;

    if (!playMove(&gameSession->game, move)) {
        char msg[] = "Error: This is not legal. Please try again\n";
        writeClient(client->sock, msg);
        return 0;
    }

    char movePlayed[BUF_SIZE] = "\0";
    sprintf(movePlayed, "%s played %d!\n", client->username, move.houseNum);
    writeClient(opponent->sock, movePlayed);

    gameSession->currentPlayer = next;

    char grid[BUF_SIZE] = "\0";
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        strcpy(usernames[i], gameSession->players[i]->username);
    }
    printGridMessage(grid, &gameSession->game, NUM_HOUSES, NUM_PLAYERS, usernames);
    writeClient(client->sock, grid);
    writeClient(opponent->sock, grid);

    if (isGameOver(&gameSession->game, NUM_PLAYERS, NUM_HOUSES)) {
        handleEndgame(gameSession);
    }

    return 1;
}

int suggestEndgame(Client *client, GameSession *gameSessions, int actualGame) {
    if (!client->gameId) {
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, gameSessions, actualGame);
    if (gameSession == NULL) {
        return 0;
    }

    if (gameSession->players[0] == client) {
        if (gameSession->endGameSuggested == 1) {
            handleEndgame(gameSession);
        }
        gameSession->endGameSuggested = 0;
    } else if (gameSession->players[1] == client) {
        if (gameSession->endGameSuggested == 0) {
            handleEndgame(gameSession);
        }
        gameSession->endGameSuggested = 1;
    }
    
    Client *opponent = gameSession->players[nextPlayer(gameSession->currentPlayer)];
    writeClient(opponent->sock, "The opponent suggests ending this game. ACCEPTEND?\n");

    return 1;
}

int acceptEndgame(Client *client, GameSession *gameSessions, int actualGame) {
        if (!client->gameId) {
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, gameSessions, actualGame);
    if (gameSession == NULL) {
        return 0;
    }

    if (gameSession->players[!gameSession->endGameSuggested] == client) {
        handleEndgame(findGameSessionByClient(client, gameSession, actualGame));
        return 1;
    }

    return 0;
}

void handleEndgame(GameSession *gameSession) {
    int winner = endGame(&gameSession->game);

    char message[BUF_SIZE] = "\0";
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        strcpy(usernames[i], gameSession->players[i]->username);
    }
    printGameEndMessage(message, &gameSession->game, NUM_PLAYERS, winner, usernames);
    for (int i = 0; i < NUM_PLAYERS; i++) {

    writeClient(gameSession->players[i]->sock, message);
    }

    for (int i = 0; i < NUM_PLAYERS; i++) {
        gameSession->players[i]->gameId = NULL;
    }

    freeGame(&gameSession->game);
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
    writeClient(requester.sock, message);
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
                     gameSessions[i].players[0]->username,
                     gameSessions[i].players[1]->username);
            strncat(message, gameInfo, BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(requester.sock, message);
}

void sendMP(Client *clients, int actual, char *username, char *message) {
    Client *client = findClientByUsername(clients, actual, username);
    if (client != NULL) {
        writeClient(client->sock, message);
    }
}

void updateBio(Client *client, char bio[]) {
    strncpy(client->bio, bio, BUF_SIZE - 1);
    char msg[] = "Bio updated successfully.\n";
    writeClient(client->sock, msg);
}

int showBio(Client *clients, int actual, Client *requester, char username[]) {
    Client *client;
    if (username == NULL || strlen(username) == 0) {
        client = requester;
    } else {
        client = findClientByUsername(clients, actual, username);
        if (client == NULL) {
            char msg[] = "Error: User not found.\n";
            writeClient(requester->sock, msg);
            return 0;
        }

        if (client->private) {
            // check if requester is a friend
            int found = 0;
            for (int i = 0; i < client->numFriends; i++) {
                if (strcmp(client->friends[i], requester->username) == 0) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                char msg[] = "Error: This user's bio is private.\n";
                writeClient(requester->sock, msg);
                return 0;
            }
        }
    }

    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "Bio of %s:\n%s", client->username, client->bio);
    writeClient(requester->sock, message);
    return 1;
}

int addFriend(Client *client, char username[]) {
    // doesn't check if user exists so that a disconnected user can still be added as a friend

    // check if already friends
    for (int i = 0; i < client->numFriends; i++) {
        if (strcmp(client->friends[i], username) == 0) {
            char msg[] = "Error: This user is already your friend.\n";
            writeClient(client->sock, msg);
            return 0;
        }
    }

    // check if friend list is full
    if (client->numFriends >= MAX_FRIENDS) {
        char msg[] = "Error: Friend list is full.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    strncpy(client->friends[client->numFriends], username, BUF_SIZE - 1);
    client->numFriends++;

    return 1;
}

void setPrivacy(Client *client, int privacy) {
    client->private = privacy;
    char msg[] = "Privacy setting updated.\n";
    writeClient(client->sock, msg);
}
