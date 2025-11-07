#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int signUp(Client *clients, int *actualClient, Client **connectedClients, int *actualConnected, SOCKET *lobby, int *actualLobby, int index, char *username, char *password) {
    if (*actualClient >= MAX_CLIENTS - 1) {
        char msg[] = "Error: Too many players.\n";
        writeClient(lobby[index], msg);
        return 0;
    }

    for (int i = 0; i < *actualClient; i++) {
        if (strcmp(clients[i].username, username) == 0) {
            char msg[] = "Error: Username is already in use.\n";
            writeClient(lobby[index], msg);
            return 0;
        }
    }

    clients[*actualClient].sock = lobby[index];
    strncpy(clients[*actualClient].username, username, BUF_SIZE - 1);
    strncpy(clients[*actualClient].password, password, BUF_SIZE - 1);
    clients[*actualClient].gameId = NULL;
    clients[*actualClient].numFriends = 0;
    clients[*actualClient].numPendingChallengesTo = 0;
    clients[*actualClient].numPendingChallengesFrom = 0;
    clients[*actualClient].bio[0] = '\0';
    clients[*actualClient].private = 0;

    connectedClients[*actualConnected] = &clients[*actualClient];
    (*actualClient)++;
    (*actualConnected)++;

    char msg[BUF_SIZE] = "\0";
    sprintf(msg, "Welcome, %s! You can now challenge your first opponent!\n", username);
    writeClient(lobby[index], msg);

    removeFromLobby(lobby, index, actualLobby);

    return 1;
}

int login(Client *clients, int *actualClient, Client **connectedClients, int *actualConnected, SOCKET *lobby, int *actualLobby, int index, char *username, char *password) {
    if (*actualConnected >= MAX_CONNECTED_CLIENTS - 1) {
        char msg[] = "Error: Too many simultaneous connections. Please wait.\n";
        writeClient(lobby[index], msg);
        return 0;
    }

    int i = 0;
    int usernameFound = 0;
    int passwordOkay = 0;

    for (i = 0; i < *actualClient; i++) {
        if (strcmp(clients[i].username, username) == 0) {
            usernameFound = 1;
            if (strcmp(clients[i].password, password) == 0) {
                passwordOkay = 1;
            }
            break;
        }
    }

    if (!usernameFound) {
        char msg[] = "Error: Username not found.\n";
        writeClient(lobby[index], msg);
        return 0;
    }

    if (usernameFound && !passwordOkay) {
        char msg[] = "Error: Wrong password.\n";
        writeClient(lobby[index], msg);
        return 0;
    }

    if (clients[i].sock >= 0) {
        char msg[] = "You've been disconnected because of a connection on another device.\n";
        writeClient(clients[i].sock, msg);
        removeClient(connectedClients, i, actualConnected);
    }

    clients[i].sock = lobby[index];

    connectedClients[*actualConnected] = &clients[i];
    (*actualConnected)++;

    char msg[BUF_SIZE] = "\0";
    sprintf(msg, "Welcome back, %s!\n", username);
    writeClient(lobby[index], msg);

    removeFromLobby(lobby, index, actualLobby);

    return 1;
}

int challenge(Client **connectedClients, Client *challenger, int actualConnected, char username[]) {
    if (strcmp(challenger->username, username) == 0) {
        char msg[] = "Error: You cannot challenge yourself.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    Client *challenged = findClientByUsername(connectedClients, actualConnected, username);
    if (challenged == NULL) {
        char msg[] = "Error: User not found.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    if (addChallenge(challenger, challenged) == 0) {
        return 0;
    }

    char message[2*BUF_SIZE];
    snprintf(message, 2*BUF_SIZE, "CHALLENGE_FROM %s", challenger->username);
    writeClient(challenged->sock, message);

    return 1;
}

int acceptChallenge(Client **connectedClients, Client *client, int actualConnected, char challenger[], GameSession *gameSessions, int *numGames, GameSession **activeGameSessions, int *numActiveGames) {
    Client *challengerClient = findClientByUsername(connectedClients, actualConnected, challenger);
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

    if (*numActiveGames > MAX_ACTIVE_GAMES - 1 || *numGames > MAX_GAMES - 1) {
        char msg[] = "Error: Too many active games. Please wait.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    if (removeChallenge(challengerClient, client) == 0) {
        return 0;
    }

    char message[2*BUF_SIZE];
    snprintf(message, 2*BUF_SIZE, "CHALLENGE_ACCEPTED_BY %s", client->username);
    writeClient(challengerClient->sock, message);

    strcpy(message, "Enter rotation (0 for counter-clockwise, 1 for clockwise): ");
    writeClient(client->sock, message);

    char rotationStr[2];
    readClient(client->sock, rotationStr);
    int rotation = atoi(rotationStr);

    Game game = startGame(rotation);

    GameSession gameSession;
    gameSession.game = game;
    gameSession.currentPlayer = playerSelector();

    gameSession.players[0] = challengerClient;
    gameSession.players[1] = client;
    gameSession.id = (int) time(NULL);  // timestamp
    gameSession.endGameSuggested = -1;
    gameSession.numViewers = 0;

    gameSessions[*numGames] = gameSession;
    activeGameSessions[*numActiveGames] = &gameSessions[*numGames];
    (*numGames)++;
    (*numActiveGames)++;

    challengerClient->gameId = &gameSessions[(*numGames)-1].id;
    client->gameId = &gameSessions[(*numGames)-1].id;

    message[0] = '\0';
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        strcpy(usernames[i], gameSession.players[i]->username);
    }
    printGridMessage(message, &gameSession.game, NUM_HOUSES, NUM_PLAYERS, usernames);
    writeClient(client->sock, message);
    writeClient(challengerClient->sock, message);
    for (int i = 0; i < gameSession.numViewers; i++) {
        writeClient(gameSession.viewers[i]->sock, message);
    }
    writeClient(gameSession.players[gameSession.currentPlayer]->sock, "It's your turn to shine!\n");

    return 1;
}

int declineChallenge(Client **connectedClients, Client *client, int actualConnected, char challenger[]) {
    Client *challengerClient = findClientByUsername(connectedClients, actualConnected, challenger);
    if (challengerClient == NULL) {
        // challenger not found
        char msg[] = "Error : challenger not found\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    removeChallenge(challengerClient, client);

    char message[2*BUF_SIZE];
    snprintf(message, 2*BUF_SIZE, "CHALLENGE_DECLINED_BY %s", client->username);
    writeClient(challengerClient->sock, message);

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

int removeSentReq(Client **connectedClients, Client *client, int actualConnected, char username[]) {
    Client *challengedClient = findClientByUsername(connectedClients, actualConnected, username);
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

    char movePlayed[2*BUF_SIZE] = "\0";
    snprintf(movePlayed, 2*BUF_SIZE, "%s played %d!\n", client->username, move.houseNum);
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
    for (int i = 0; i < gameSession->numViewers; i++) {
        writeClient(gameSession->viewers[i]->sock, grid);
    }

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

void listClients(Client **connectedClients, int actualConnected, Client requester) {
    char message[BUF_SIZE];
    message[0] = '\0';
    strncat(message, "Connected users:\n", BUF_SIZE - strlen(message) - 1);

    // Find the maximum username length for formatting
    int maxLen = 0;
    for (int j = 0; j < actualConnected; j++) {
        int currLen = (int) strlen(connectedClients[j]->username);
        if (currLen > maxLen) {
            maxLen = currLen;
        }
    }

    for (int i = 0; i < actualConnected; i++) {
        strncat(message, connectedClients[i]->username, BUF_SIZE - strlen(message) - 1);

        int pad = maxLen - (int)strlen(connectedClients[i]->username) + 1;
        if (pad < 1) {
            pad = 1;
        }

        char spaces[BUF_SIZE];
        memset(spaces, ' ', (size_t)pad);
        spaces[pad] = '\0';
        strncat(message, spaces, BUF_SIZE - strlen(message) - 1);

        if (connectedClients[i]->gameId != NULL) {
            strncat(message, "in game", BUF_SIZE - strlen(message) - 1);
        }

        strncat(message, "\n",   BUF_SIZE - strlen(message) - 1);
    }
    writeClient(requester.sock, message);
}

void listGames(GameSession **gameSessions, int actualGame, Client requester) {
    char message[BUF_SIZE];
    message[0] = '\0';

    if (actualGame == 0) {
        strncat(message, "No ongoing games.\n", BUF_SIZE - strlen(message) - 1);
    } else {
        strncat(message, "Ongoing games:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < actualGame; i++) {
            char gameInfo[3*BUF_SIZE];
            snprintf(gameInfo, 3*BUF_SIZE, "Game ID: %d | Players: %s vs %s\n",
                     gameSessions[i]->id,
                     gameSessions[i]->players[0]->username,
                     gameSessions[i]->players[1]->username);
            strncat(message, gameInfo, BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(requester.sock, message);
}

int watchGame(Client *client, GameSession **gameSessions, int actualGame, int gameId) {
    if (client->gameId != NULL) {
        char msg[] = "Error: You cannot watch a game while playing in one.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    GameSession *gameSession = NULL;
    for (int i = 0; i < actualGame; i++) {
        if (gameSessions[i]->id == gameId) {
            gameSession = gameSessions[i];
            break;
        }
    }

    if (gameSession == NULL) {
        char msg[] = "Error: Game not found.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (gameSession->numViewers >= MAX_VIEWERS) {
        char msg[] = "Error: Maximum number of viewers reached for this game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    int private = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (gameSession->players[i]->private) {
            private = 1;
            break;;
        }
    }

    if (private) {
        int found = 0;
        for (int i = 0; i < NUM_PLAYERS; i++) {
            Client *player = gameSession->players[i];

            for (int j = 0; j < player->numFriends; j++) {
                if (strcmp(player->friends[j], client->username) == 0) {
                    found = 1;
                    break;
                }
            }
        }

        if (!found) {
            char msg[] = "Error: You cannot watch this private game.\n";
            writeClient(client->sock, msg);
            return 0;
        }
    }

    gameSession->viewers[gameSession->numViewers] = client;
    gameSession->numViewers++;

    char msg[] = "You are now watching the game.\n";
    writeClient(client->sock, msg);

    return 1;
}

int SendMsgGame(GameSession *gameSession, Client *sender, char *message) {
    if (gameSession == NULL) {
        writeClient(sender->sock, "Error: You are not watching or playing any game.\n");
        return 0;
    }

    int found = 0;
    for (int i = 0; i < gameSession->numViewers; i++) {
        Client *viewer = gameSession->viewers[i];
        if (viewer->username == sender->username) {
            found = 1;
            break;
        }
    }

    if (!found && sender->gameId != NULL && *(sender->gameId) != gameSession->id) {
        char msg[] = "Error: You are not part of this game.\n";
        writeClient(sender->sock, msg);
        return 0;
    }

    // Format message to add sender's name
    char formattedMessage[2*BUF_SIZE];
    snprintf(formattedMessage, 2*BUF_SIZE, "%s (game chat): %s\n", sender->username, message);

    // Send to viewers
    for (int i = 0; i < gameSession->numViewers; i++) {
        Client *recipient = gameSession->viewers[i];
        writeClient(recipient->sock, formattedMessage);
    }

    // Send to players
    for (int i = 0; i < NUM_PLAYERS; i++) {
        Client *player = gameSession->players[i];
        writeClient(player->sock, formattedMessage);
    }

    return 1;
}

void sendMP(Client **connectedClients, Client *sender, int actualConnected, char *username, char *message) {
    Client *client = findClientByUsername(connectedClients, actualConnected, username);

    // Format message to add sender's name
    char formattedMessage[2*BUF_SIZE];
    snprintf(formattedMessage, 2*BUF_SIZE, "%s (private): %s\n", sender->username, message);

    if (client != NULL) {
        writeClient(client->sock, formattedMessage);
    }
}

void updateBio(Client *client, char bio[]) {
    strncpy(client->bio, bio, BUF_SIZE - 1);
    char msg[] = "Bio updated successfully.\n";
    writeClient(client->sock, msg);
}

int showBio(Client **connectedClients, int actualConnected, Client *requester, char username[]) {
    Client *client;
    if (username == NULL || strlen(username) == 0) {
        client = requester;
    } else {
        client = findClientByUsername(connectedClients, actualConnected, username);
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

    char message[3*BUF_SIZE];
    snprintf(message, 3*BUF_SIZE, "Bio of %s:\n%s", client->username, client->bio);
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

int quit(Client **connectedClients, int *actualConnected, Client *client, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames) {
    // If the client is in a game, handle game termination
    if (client->gameId != NULL) {
        GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);
        if (gameSession == NULL) {
            char msg[] = "Error: You're not currently in a game.\n";
            writeClient(client->sock, msg);
            return 0;
        }

        client->gameId = NULL;

        // Notify the opponent
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (gameSession->players[i] != client) {
                char msg[BUF_SIZE];
                snprintf(msg, BUF_SIZE, "The opponent %s has disconnected. The game has been saved.\n", client->username);
                writeClient(gameSession->players[i]->sock, msg);
                gameSession->players[i]->gameId = NULL;
            }
        }

        removeActiveGameSession(activeGameSessions, numActiveGames, gameSession->id);
    }


    closesocket(client->sock);
    client->sock = -1;

    char buffer[BUF_SIZE];
    strncpy(buffer, client->username, BUF_SIZE - 1);
    strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);

    int i = findClientIndex(connectedClients, *actualConnected, client);
    removeClient(connectedClients, i, actualConnected);

    sendMessageToAllClients(connectedClients, client->username, *actualConnected, buffer, 1);

    return 1;
}

int loadGame(Client **connectedClients, int actualConnected, Client *client, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames) {
    if (*numGames == 0) {
        char msg[] = "Error: No saved games to load. Next time, quit by using the QUIT command.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    int clientLastSavedGameIndex = -1;
    for (int i = 0; i < *numGames; i++) {
        // Check if the client is a player in the saved game
        for (int j = 0; j < NUM_PLAYERS; j++) {
            if (gameSessions[i].players[j] == client) {
                clientLastSavedGameIndex = i;
                break;
            }
        }
    }

    if (clientLastSavedGameIndex == -1) {
        char msg[] = "Error: You are not a player in any saved game. Next time, quit by using the QUIT command.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Check if both players are connected
    for (int i = 0; i < NUM_PLAYERS; i++) {
        Client *player = gameSessions[clientLastSavedGameIndex].players[i];

        if (player == client) {
            continue;
        }

        for (int j = 0; j < actualConnected; j++) {
            if (connectedClients[j] == player) {
                break;
            }
            if (j == actualConnected - 1) {
                char msg[] = "Error: Both players must be connected to load the saved game.\n";
                writeClient(client->sock, msg);
                return 0;
            }
        }
    }

    GameSession *gameSession = &gameSessions[clientLastSavedGameIndex];

    activeGameSessions[*numActiveGames] = gameSession;
    (*numActiveGames)++;

    char msg[] = "Last saved game loaded successfully.\n";
    writeClient(client->sock, msg);

    Client *challengerClient = NULL;
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        strcpy(usernames[i], gameSession->players[i]->username);

        if (gameSession->players[i] != client) {
            challengerClient = gameSession->players[i];
        }
    }

    // Set gameId for both players
    challengerClient->gameId = &gameSession->id;
    client->gameId = &gameSession->id;

    char msgGrid[BUF_SIZE] = "\0";
    printGridMessage(msgGrid, &gameSession->game, NUM_HOUSES, NUM_PLAYERS, usernames);
    writeClient(client->sock, msgGrid);
    writeClient(challengerClient->sock, msgGrid);
    for (int i = 0; i < gameSession->numViewers; i++) {
        writeClient(gameSession->viewers[i]->sock, msgGrid);
    }
    writeClient(gameSession->players[gameSession->currentPlayer]->sock, "It's your turn to shine!\n");

    return 1;
}
