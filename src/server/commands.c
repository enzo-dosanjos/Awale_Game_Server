#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// For saveGameAndSend
#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(path) _mkdir(path)
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #define MKDIR(path) mkdir(path, 0755)
#endif


int signUp(Client *clients, int *actualClient, Client **connectedClients, int *actualConnected, SOCKET *lobby, int *actualLobby, int index, char *username, char *password)
{
    if (*actualClient >= MAX_CLIENTS - 1)
    {
        char msg[] = "Error: Too many players.\n";
        writeClient(lobby[index], msg);
        return 0;
    }

    for (int i = 0; i < *actualClient; i++)
    {
        if (strcmp(clients[i].username, username) == 0)
        {
            char msg[] = "Error: Username is already in use.\n";
            writeClient(lobby[index], msg);
            return 0;
        }
    }

    initClient(clients, actualClient, lobby[index], username, password);

    connectedClients[*actualConnected] = &clients[*actualClient];
    (*actualClient)++;
    (*actualConnected)++;

    char msg[BUF_SIZE] = "\0";
    sprintf(msg, "Welcome, %s! You can now challenge your first opponent!\n", username);
    writeClient(lobby[index], msg);

    removeFromLobby(lobby, index, actualLobby);

    return 1;
}

int login(Client *clients, int *actualClient, Client **connectedClients, int *actualConnected, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames, SOCKET *lobby, int *actualLobby, int index, char *username, char *password)
{
    if (*actualConnected >= MAX_CONNECTED_CLIENTS - 1)
    {
        char msg[] = "Error: Too many simultaneous connections. Please wait.\n";
        writeClient(lobby[index], msg);
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
        writeClient(lobby[index], msg);
        return 0;
    }

    if (usernameFound && !passwordOkay)
    {
        char msg[] = "Error: Wrong password.\n";
        writeClient(lobby[index], msg);
        return 0;
    }

    if (clients[i].sock >= 0)
    {
        char msg[] = "You've been disconnected because of a connection on another device.\n";
        writeClient(clients[i].sock, msg);
        quit(connectedClients, actualConnected, &clients[i], activeGameSessions, numActiveGames, gameSessions, numGames);
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

int challenge(Client **connectedClients, Client *challenger, int actualConnected, char username[])
{
    if (strcmp(challenger->username, username) == 0)
    {
        char msg[] = "Error: You cannot challenge yourself.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    Client *challenged = findConnectedClientByUsername(connectedClients, actualConnected, username);
    if (challenged == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    if (addChallenge(challenger, challenged) == 0)
    {
        return 0;
    }

    char message[2 * BUF_SIZE];
    snprintf(message, 2 * BUF_SIZE, "CHALLENGE_FROM %s", challenger->username);
    writeClient(challenged->sock, message);

    return 1;
}

int acceptChallenge(Client **connectedClients, Client *client, int actualConnected, char challenger[], GameSession *gameSessions, int *numGames, GameSession **activeGameSessions, int *numActiveGames)
{
    Client *challengerClient = findConnectedClientByUsername(connectedClients, actualConnected, challenger);
    if (challengerClient == NULL)
    {
        // challenger not found
        char msg[] = "Error : challenger not found\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (client->gameId != NULL)
    {
        char msg[] = "Error: You are already in a game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (challengerClient->gameId != NULL)
    {
        char msg[] = "Error: Challenger is already in a game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (*numActiveGames > MAX_ACTIVE_GAMES - 1 || *numGames > MAX_GAMES - 1)
    {
        char msg[] = "Error: Too many active games. Please wait.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    if (removeChallenge(challengerClient, client) == 0)
    {
        return 0;
    }

    char message[2 * BUF_SIZE];
    snprintf(message, 2 * BUF_SIZE, "CHALLENGE_ACCEPTED_BY %s", client->username);
    writeClient(challengerClient->sock, message);

    strcpy(message, "Enter rotation (0 for counter-clockwise, 1 for clockwise): ");
    writeClient(client->sock, message);

    char rotationStr[2];
    readClient(client->sock, rotationStr);
    int rotation = atoi(rotationStr);

    Game game = startGame(rotation);
    int firstPlayer = playerSelector();
    GameSession *gameSession = initGameSession(gameSessions, numGames, &game, firstPlayer, challengerClient, client);

    activeGameSessions[*numActiveGames] = gameSession;
    (*numActiveGames)++;

    challengerClient->gameId = &gameSession->id;
    client->gameId = &gameSession->id;

    char grid[BUF_SIZE] = "\0";
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        strcpy(usernames[i], gameSession->players[i]->username);
    }
    printGridMessage(grid, &gameSession->game, NUM_HOUSES, NUM_PLAYERS, usernames);
    writeClient(client->sock, grid);
    writeClient(challengerClient->sock, grid);
    for (int i = 0; i < gameSession->numViewers; i++)
    {
        writeClient(gameSession->viewers[i]->sock, grid);
    }
    writeClient(gameSession->players[gameSession->currentPlayer]->sock, "It's your turn to shine!\n");

    // Save game state for history
    recordMove(gameSession, NULL, grid);

    return 1;
}

int declineChallenge(Client *clients, Client *client, int actualClients, char challenger[])
{
    Client *challengerClient = findClientByUsername(clients, actualClients, challenger);
    if (challengerClient == NULL)
    {
        // challenger not found
        char msg[] = "Error : challenger not found\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    removeChallenge(challengerClient, client);

    char message[2 * BUF_SIZE];
    snprintf(message, 2 * BUF_SIZE, "CHALLENGE_DECLINED_BY %s", client->username);
    writeClient(challengerClient->sock, message);

    return 1;
}

void seePendingReq(Client *client)
{
    char message[BUF_SIZE];
    message[0] = '\0';

    if (client->numPendingChallengesFrom == 0)
    {
        strncat(message, "No pending challenges.\n", BUF_SIZE - strlen(message) - 1);
    }
    else
    {
        strncat(message, "Pending challenges from:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < client->numPendingChallengesFrom; i++)
        {
            strncat(message, client->pendingChallengesFrom[i], BUF_SIZE - strlen(message) - 1);
            strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(client->sock, message);
}

void seeSentReq(Client *client)
{
    char message[BUF_SIZE];
    message[0] = '\0';

    if (client->numPendingChallengesTo == 0)
    {
        strncat(message, "No sent challenges.\n", BUF_SIZE - strlen(message) - 1);
    }
    else
    {
        strncat(message, "Sent challenges to:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < client->numPendingChallengesTo; i++)
        {
            strncat(message, client->pendingChallengesTo[i], BUF_SIZE - strlen(message) - 1);
            strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(client->sock, message);
}

void clearPendingReq(Client *client)
{
    clearReceivedChallenge(client);
    char msg[] = "All received pending challenges cleared.\n";
    writeClient(client->sock, msg);
}

void clearSentReq(Client *client)
{
    clearSentChallenge(client);
    char msg[] = "All sent pending challenges cleared.\n";
    writeClient(client->sock, msg);
}

int removeSentReq(Client *clients, Client *client, int actualClient, char username[])
{
    Client *challengedClient = findClientByUsername(clients, actualClient, username);
    if (challengedClient == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (removeChallenge(client, challengedClient))
    {
        char msg[] = "Pending challenge removed.\n";
        writeClient(client->sock, msg);
    }

    return 1;
}

int move(Client *client, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames, int house)
{
    if (!client->gameId)
    {
        char msg[] = "Error: You are not currently in a game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);
    if (gameSession == NULL)
    {
        return 0;
    }

    Move move;
    move.houseNum = house;

    if (client != gameSession->players[gameSession->currentPlayer])
    {
        char msg[] = "Error: It's not your turn, please wait for the opponent to make their move.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    int next = nextPlayer(gameSession->currentPlayer);
    Client *opponent = gameSession->players[next];
    move.numPlayer = gameSession->currentPlayer;

    if (!playMove(&gameSession->game, move))
    {
        char msg[] = "Error: This is not legal. Please try again\n";
        writeClient(client->sock, msg);
        return 0;
    }

    char movePlayed[2 * BUF_SIZE] = "\0";
    snprintf(movePlayed, 2 * BUF_SIZE, "%s played %d!\n", client->username, move.houseNum);
    writeClient(opponent->sock, movePlayed);

    gameSession->currentPlayer = next;

    char grid[BUF_SIZE] = "\0";
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        strcpy(usernames[i], gameSession->players[i]->username);
    }
    printGridMessage(grid, &gameSession->game, NUM_HOUSES, NUM_PLAYERS, usernames);
    writeClient(client->sock, grid);
    writeClient(opponent->sock, grid);
    for (int i = 0; i < gameSession->numViewers; i++)
    {
        writeClient(gameSession->viewers[i]->sock, grid);
    }

    // Save game state for history
    recordMove(gameSession, &move, grid);

    gameSession->numMoves++;

    if (isGameOver(&gameSession->game, NUM_PLAYERS, NUM_HOUSES))
    {
        handleEndgame(gameSession, activeGameSessions, numActiveGames, gameSessions, numGames);
    }

    return 1;
}

int suggestEndgame(Client *client, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames)
{
    if (!client->gameId)
    {
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);
    if (gameSession == NULL)
    {
        return 0;
    }

    if (gameSession->players[0] == client)
    {
        if (gameSession->endGameSuggested == 1)
        {
            handleEndgame(gameSession, activeGameSessions, numActiveGames, gameSessions, numGames);
        }
        gameSession->endGameSuggested = 0;
    }
    else if (gameSession->players[1] == client)
    {
        if (gameSession->endGameSuggested == 0)
        {
            handleEndgame(gameSession, activeGameSessions, numActiveGames, gameSessions, numGames);
        }
        gameSession->endGameSuggested = 1;
    }

    Client *opponent = gameSession->players[nextPlayer(gameSession->endGameSuggested)];
    writeClient(opponent->sock, "The opponent suggests ending this game. ACCEPTEND?\n");

    return 1;
}

int acceptEndgame(Client *client, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames)
{
    if (!client->gameId)
    {
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);
    if (gameSession == NULL)
    {
        return 0;
    }

    if (gameSession->players[!gameSession->endGameSuggested] == client)
    {
        handleEndgame(gameSession, activeGameSessions, numActiveGames, gameSessions, numGames);
        return 1;
    }

    return 0;
}

void handleEndgame(GameSession *gameSession, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames)
{
    int winner = endGame(&gameSession->game);

    // Ask the players if they want to save the game
    char saveMsg[] = "The game has ended. Do you want to save the game? (yes/no): ";
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        writeClient(gameSession->players[i]->sock, saveMsg);

        char response[BUF_SIZE];
        readClient(gameSession->players[i]->sock, response);

        if (strcmp(response, "yes") == 0 || strcmp(response, "y") == 0)
        {
            saveGameAndSend(gameSession->players[i], activeGameSessions, *numActiveGames);
        }
    }

    char message[BUF_SIZE] = "\0";
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        strcpy(usernames[i], gameSession->players[i]->username);
    }
    printGameEndMessage(message, &gameSession->game, NUM_PLAYERS, winner, usernames);
    for (int i = 0; i < NUM_PLAYERS; i++)
    {

        writeClient(gameSession->players[i]->sock, message);
    }

    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        gameSession->players[i]->gameId = NULL;
        gameSession->players[i]->stats.gamesPlayed++;

        if (winner == -1)
        {
            gameSession->players[i]->stats.gamesDrawn++;
        }
        else if (i == winner)
        {
            double prevAvg = gameSession->players[i]->stats.averageMovesToWin;
            gameSession->players[i]->stats.averageMovesToWin = (prevAvg * gameSession->players[i]->stats.gamesWon + gameSession->numMoves / NUM_PLAYERS) / (gameSession->players[i]->stats.gamesWon + 1);
            gameSession->players[i]->stats.gamesWon++;
        }
        else
        {
            gameSession->players[i]->stats.gamesLost++;
        }

        gameSession->players[i]->stats.totalSeedsCollected += gameSession->game.scores[i];
    }

    removeActiveGameSession(activeGameSessions, numActiveGames, gameSession->id);
    removeGameSession(gameSessions, numGames, gameSession->id);
    freeGame(&gameSession->game);
}

void listClients(Client **connectedClients, int actualConnected, Client requester)
{
    char message[BUF_SIZE];
    message[0] = '\0';
    strncat(message, "Connected users:\n", BUF_SIZE - strlen(message) - 1);

    // Find the maximum username length for formatting
    int maxLen = 0;
    for (int j = 0; j < actualConnected; j++)
    {
        int currLen = (int)strlen(connectedClients[j]->username);
        if (currLen > maxLen)
        {
            maxLen = currLen;
        }
    }

    for (int i = 0; i < actualConnected; i++)
    {
        strncat(message, connectedClients[i]->username, BUF_SIZE - strlen(message) - 1);

        int pad = maxLen - (int)strlen(connectedClients[i]->username) + 1;
        if (pad < 1)
        {
            pad = 1;
        }

        char spaces[BUF_SIZE];
        memset(spaces, ' ', (size_t)pad);
        spaces[pad] = '\0';
        strncat(message, spaces, BUF_SIZE - strlen(message) - 1);

        if (connectedClients[i]->gameId != NULL)
        {
            strncat(message, "in game", BUF_SIZE - strlen(message) - 1);
        }

        strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
    }
    writeClient(requester.sock, message);
}

void listGames(GameSession **gameSessions, int actualGame, Client requester)
{
    char message[BUF_SIZE];
    message[0] = '\0';

    if (actualGame == 0)
    {
        strncat(message, "No ongoing games.\n", BUF_SIZE - strlen(message) - 1);
    }
    else
    {
        strncat(message, "Ongoing games:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < actualGame; i++)
        {
            char gameInfo[3 * BUF_SIZE];
            snprintf(gameInfo, 3 * BUF_SIZE, "Game ID: %d | Players: %s vs %s\n",
                     gameSessions[i]->id,
                     gameSessions[i]->players[0]->username,
                     gameSessions[i]->players[1]->username);
            strncat(message, gameInfo, BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(requester.sock, message);
}

int watchGame(Client *client, GameSession **gameSessions, int actualGame, int gameId)
{
    if (client->gameId != NULL)
    {
        char msg[] = "Error: You cannot watch a game while playing in one.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    GameSession *gameSession = NULL;
    for (int i = 0; i < actualGame; i++)
    {
        if (gameSessions[i]->id == gameId)
        {
            gameSession = gameSessions[i];
            break;
        }
    }

    if (gameSession == NULL)
    {
        char msg[] = "Error: Game not found.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (gameSession->numViewers >= MAX_VIEWERS)
    {
        char msg[] = "Error: Maximum number of viewers reached for this game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    int private = 0;
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        if (gameSession->players[i]->private)
        {
            private = 1;
            break;
            ;
        }
    }

    if (private)
    {
        int found = 0;
        for (int i = 0; i < NUM_PLAYERS; i++)
        {
            Client *player = gameSession->players[i];

            for (int j = 0; j < player->numFriends; j++)
            {
                if (strcmp(player->friends[j], client->username) == 0)
                {
                    found = 1;
                    break;
                }
            }
        }

        if (!found)
        {
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

int SendMsgGame(GameSession *gameSession, Client *sender, char *message)
{
    if (gameSession == NULL)
    {
        writeClient(sender->sock, "Error: You are not watching or playing any game.\n");
        return 0;
    }

    int found = 0;
    for (int i = 0; i < gameSession->numViewers; i++)
    {
        Client *viewer = gameSession->viewers[i];
        if (viewer->username == sender->username)
        {
            found = 1;
            break;
        }
    }

    if (!found && sender->gameId != NULL && *(sender->gameId) != gameSession->id)
    {
        char msg[] = "Error: You are not part of this game.\n";
        writeClient(sender->sock, msg);
        return 0;
    }

    // Format message to add sender's name
    char formattedMessage[2 * BUF_SIZE];
    snprintf(formattedMessage, 2 * BUF_SIZE, "%s (game chat): %s\n", sender->username, message);

    // Send to viewers
    for (int i = 0; i < gameSession->numViewers; i++)
    {
        Client *recipient = gameSession->viewers[i];
        writeClient(recipient->sock, formattedMessage);
    }

    // Send to players
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        Client *player = gameSession->players[i];
        writeClient(player->sock, formattedMessage);
    }

    // Log the message in game history
    recordChat(gameSession, sender->username, message);

    return 1;
}

void sendMP(Client **connectedClients, Client *sender, int actualConnected, char *username, char *message)
{
    Client *client = findConnectedClientByUsername(connectedClients, actualConnected, username);
    if (client == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(sender->sock, msg);
        return;
    }

    // Format message to add sender's name
    char formattedMessage[2 * BUF_SIZE];
    snprintf(formattedMessage, 2 * BUF_SIZE, "%s (private): %s\n", sender->username, message);

    writeClient(client->sock, formattedMessage);
}

void updateBio(Client *client, char bio[])
{
    strncpy(client->bio, bio, BUF_SIZE - 1);
    char msg[] = "Bio updated successfully.\n";
    writeClient(client->sock, msg);
}

int showBio(Client *clients, int actualClient, Client *requester, char username[])
{
    Client *client;
    if (username == NULL || strlen(username) == 0)
    {
        client = requester;
    }
    else
    {
        client = findClientByUsername(clients, actualClient, username);
        if (client == NULL)
        {
            char msg[] = "Error: User not found.\n";
            writeClient(requester->sock, msg);
            return 0;
        }

        if (client->private)
        {
            // check if requester is a friend
            int found = 0;
            for (int i = 0; i < client->numFriends; i++)
            {
                if (strcmp(client->friends[i], requester->username) == 0)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                char msg[] = "Error: This user's bio is private.\n";
                writeClient(requester->sock, msg);
                return 0;
            }
        }
    }

    char message[3 * BUF_SIZE];
    snprintf(message, 3 * BUF_SIZE, "Bio of %s:\n%s", client->username, client->bio);
    writeClient(requester->sock, message);
    return 1;
}

int showStats(Client *clients, int actualClient, Client *requester, char username[])
{
    Client *client;
    if (username == NULL || strlen(username) == 0)
    {
        client = requester;
    }
    else
    {
        client = findClientByUsername(clients, actualClient, username);
        if (client == NULL)
        {
            char msg[] = "Error: User not found.\n";
            writeClient(requester->sock, msg);
            return 0;
        }

        if (client->private)
        {
            // check if requester is a friend
            int found = 0;
            for (int i = 0; i < client->numFriends; i++)
            {
                if (strcmp(client->friends[i], requester->username) == 0)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                char msg[] = "Error: This user's bio is private.\n";
                writeClient(requester->sock, msg);
                return 0;
            }
        }
    }

    char message[3 * BUF_SIZE];
    snprintf(message, 3 * BUF_SIZE, "Stats of %s:\n\tGames played: %d\n\tGames won: %d\n\tGames lost: %d\n\tGames drawn: %d\n\tAverage number of moves to win: %.0f\n\tTotal number of seeds collected: %d\n", client->username, client->stats.gamesPlayed, client->stats.gamesWon, client->stats.gamesLost, client->stats.gamesDrawn, client->stats.averageMovesToWin, client->stats.totalSeedsCollected);
    writeClient(requester->sock, message);
    return 1;
}

int addFriend(Client *clients, int actualClient, Client *client, char username[])
{
    if (strcmp(client->username, username) == 0)
    {
        char msg[] = "Error: You cannot add yourself as a friend.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    Client *friend = findClientByUsername(clients, actualClient, username);
    if (friend == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // check if already friends
    for (int i = 0; i < client->numFriends; i++)
    {
        if (strcmp(client->friends[i], username) == 0)
        {
            char msg[] = "Error: This user is already your friend.\n";
            writeClient(client->sock, msg);
            return 0;
        }
    }

    // check if friend list is full
    if (client->numFriends >= MAX_FRIENDS)
    {
        char msg[] = "Error: Friend list is full.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    strncpy(client->friends[client->numFriends], username, BUF_SIZE - 1);
    client->numFriends++;

    return 1;
}

int removeFriend(Client *client, char username[])
{
    int found = 0;
    for (int i = 0; i < client->numFriends; i++)
    {
        if (strcmp(client->friends[i], username) == 0)
        {
            found = 1;
            // shift friends down
            for (int j = i; j < client->numFriends - 1; j++)
            {
                strcpy(client->friends[j], client->friends[j + 1]);
            }
            client->numFriends--;
            break;
        }
    }

    if (!found)
    {
        char msg[] = "Error: This user is not your friend.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    return 1;
}

int showFriends(Client *clients, int actualClient, Client *requester, char username[])
{
    Client *client;
    if (username == NULL || strlen(username) == 0)
    {
        client = requester;
    }
    else
    {
        client = findClientByUsername(clients, actualClient, username);
        if (client == NULL)
        {
            char msg[] = "Error: User not found.\n";
            writeClient(requester->sock, msg);
            return 0;
        }

        if (client->private)
        {
            // check if requester is a friend
            int found = 0;
            for (int i = 0; i < client->numFriends; i++)
            {
                if (strcmp(client->friends[i], requester->username) == 0)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                char msg[] = "Error: This user's friends list is private.\n";
                writeClient(requester->sock, msg);
                return 0;
            }
        }
    }

    char message[2*BUF_SIZE];

    sprintf(message, "Friends of %s:\n", client->username);
    if (client->numFriends == 0)
    {
        strcat(message, "No friends added yet.\n");
    }
    else
    {
        for (int i = 0; i < client->numFriends; i++)
        {
            strcat(message, client->friends[i]);
            strcat(message, "\n");
        }
    }

    writeClient(requester->sock, message);
    return 1;
}

void setPrivacy(Client *client, int privacy)
{
    client->private = privacy;
    char msg[] = "Privacy setting updated.\n";
    writeClient(client->sock, msg);
}

int quit(Client **connectedClients, int *actualConnected, Client *client, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames)
{
    // If the client is in a game, handle game termination
    if (client->gameId != NULL)
    {
        GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);
        if (gameSession != NULL)
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

        client->gameId = NULL;
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

int loadGame(Client **connectedClients, int actualConnected, Client *client, GameSession **activeGameSessions, int *numActiveGames, GameSession *gameSessions, int *numGames)
{
    if (*numGames == 0)
    {
        char msg[] = "Error: No saved games to load. Next time, quit by using the QUIT command.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (client->gameId != NULL)
    {
        char msg[] = "Error: You are already in a game. You cannot load another game right now.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    int clientLastSavedGameIndex = -1;
    for (int i = 0; i < *numGames; i++)
    {
        // Check if the client is a player in the saved game
        for (int j = 0; j < NUM_PLAYERS; j++)
        {
            if (gameSessions[i].players[j] == client)
            {
                clientLastSavedGameIndex = i;
                break;
            }
        }
    }

    if (clientLastSavedGameIndex == -1)
    {
        char msg[] = "Error: You are not a player in any saved game. Next time, quit by using the QUIT command.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Check if both players are connected
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        Client *player = gameSessions[clientLastSavedGameIndex].players[i];

        if (player == client)
        {
            continue;
        }
        else if (player->gameId != NULL)
        {
            char msg[] = "Error: Your opponent is already in a game. You can't resume this one right now.\n";
            writeClient(client->sock, msg);
            return 0;
        }

        int j = 0;
        for (j = 0; j < actualConnected; j++)
        {
            if (connectedClients[j] == player)
            {
                break;
            }
        }
        if (j == actualConnected)
        {
            char msg[] = "Error: Both players must be connected to load the saved game.\n";
            writeClient(client->sock, msg);
            return 0;
        }
    }

    GameSession *gameSession = &gameSessions[clientLastSavedGameIndex];

    activeGameSessions[*numActiveGames] = gameSession;
    (*numActiveGames)++;

    char msg[] = "Last saved game loaded successfully.\n";
    writeClient(client->sock, msg);

    Client *challengerClient = NULL;
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        strcpy(usernames[i], gameSession->players[i]->username);

        if (gameSession->players[i] != client)
        {
            challengerClient = gameSession->players[i];
        }
    }

    // Set gameId for both players
    challengerClient->gameId = &gameSession->id;
    client->gameId = &gameSession->id;

    char msgStart[3 * BUF_SIZE] = "\0";
    snprintf(msgStart, 3 * BUF_SIZE, "Game between %s and %s has resumed!\n", challengerClient->username, client->username);
    writeClient(client->sock, msgStart);
    writeClient(challengerClient->sock, msgStart);

    char msgGrid[BUF_SIZE] = "\0";
    printGridMessage(msgGrid, &gameSession->game, NUM_HOUSES, NUM_PLAYERS, usernames);
    writeClient(client->sock, msgGrid);
    writeClient(challengerClient->sock, msgGrid);
    for (int i = 0; i < gameSession->numViewers; i++)
    {
        writeClient(gameSession->viewers[i]->sock, msgGrid);
    }

    writeClient(gameSession->players[gameSession->currentPlayer]->sock, "It's your turn to shine!\n");

    return 1;
}

void sendHelp(SOCKET sock, int loggedIn)
{
    char out[8 * BUF_SIZE];
    out[0] = '\0';

    strcat(out, "Available commands:");
    strcat(out, "\n");

    if (!loggedIn)
    {
        strcat(out, "\n");
        strcat(out, "Lobby:\n");
        strcat(out, "  HELP                                   - Show this help.\n");
        strcat(out, "  MSG <message>                          - Send a message to the lobby.\n");
        strcat(out, "  LOGIN <username> <password>            - Log into an existing account.\n");
        strcat(out, "  SIGNUP <username> <password>           - Create a new account and connect.\n");
    }
    else
    {
        strcat(out, "\n");
        strcat(out, "General:\n");
        strcat(out, "  HELP                                   - Show this help.\n");
        strcat(out, "  LIST                                   - List connected users and if they are currently playing.\n");
        strcat(out, "  LISTGAMES                              - List ongoing games.\n");
        strcat(out, "  QUIT                                   - Disconnect.\n");

        strcat(out, "\n");
        strcat(out, "Challenges:\n");
        strcat(out, "  CHALLENGE <username>                   - Send a game challenge.\n");
        strcat(out, "  ACCEPT <username>                      - Accept a pending challenge.\n");
        strcat(out, "  DECLINE <username>                     - Decline a pending challenge.\n");
        strcat(out, "  SEEPENDINGREQ                          - List received challenges.\n");
        strcat(out, "  SEESENTREQ                             - List sent challenges.\n");
        strcat(out, "  CLEARPENDINGREQ                        - Clear received challenges.\n");
        strcat(out, "  CLEARSENTREQ                           - Clear sent challenges.\n");
        strcat(out, "  REMOVESENTREQ <username>               - Unsend a challenge.\n");

        strcat(out, "\n");
        strcat(out, "Messaging:\n");
        strcat(out, "  MSG <message>                          - Send a message to the general chat.\n");
        strcat(out, "  MSG @<username> <message>              - Send a private message.\n");

        strcat(out, "\n");
        strcat(out, "Game:\n");
        strcat(out, "  MSGGAME <message>                      - Send a message to the current game chat.\n");
        strcat(out, "  MOVE <house>                           - Play a move when it's your turn.\n");
        strcat(out, "  ENDGAME                                - Propose to end the current game.\n");
        strcat(out, "  ACCEPTEND                              - Accept the endgame proposal.\n");
        strcat(out, "  LASTGAME                               - Load your last unfinished game.\n");
        strcat(out, "  SAVEGAME                               - Save the current game.\n");

        strcat(out, "\n");
        strcat(out, "Spectating:\n");
        strcat(out, "  LISTGAMES                              - List ongoing games ids.\n");
        strcat(out, "  WATCH <gameId>                         - Watch a game (respects privacy).\n");
        strcat(out, "  MSGGAME <message>                      - Send a message to the watched game chat.\n");

        strcat(out, "\n");
        strcat(out, "Profile:\n");
        strcat(out, "  BIO <text>                             - Update your bio.\n");
        strcat(out, "  SHOWBIO [username]                     - Show your or someone's bio (respects privacy).\n");
        strcat(out, "  SHOWSTATS [username]                   - Show your or someone's stats (respects privacy).\n");
        strcat(out, "  ADDFRIEND <username>                   - Add a friend.\n");
        strcat(out, "  REMOVEFRIEND <username>                - Remove a friend.\n");
        strcat(out, "  SHOWFRIENDS [username]                 - Show your or someone's friends list (respects privacy).\n");
        strcat(out, "  SETPRIVACY <true|false>                - true makes your bio/game private to friends.\n");
    }

    writeClient(sock, out);
}

int saveGameAndSend(Client *client, GameSession **activeGameSessions, int numActiveGames)
{
    if (!client->gameId) {
        writeClient(client->sock, "Error: You are not currently in a game.\n");
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, numActiveGames);
    if (!gameSession) {
        return 0;
    }

    // Create the save dir and file
    MKDIR(SAVE_DIR);
    char filepath[2*BUF_SIZE];
    char filename[BUF_SIZE];
    sprintf(filename, "game_%d.txt", gameSession->id);
    sprintf(filepath, "%s/%s", SAVE_DIR, filename);


    // check if the file already exists (created for the other player)
    int already_saved = 0;
    FILE *fcheck = fopen(filepath, "rb");
    if (fcheck) {
        already_saved = 1;
        fclose(fcheck);
    }

    if (!already_saved)
    {
        FILE *file = fopen(filepath, "wb");
        if (!file) {
            writeClient(client->sock, "Error: Could not save the game.\n");
            return 0;
        }

        // header
        fprintf(file, "Full game export\n");
        fprintf(file, "Game ID: %d\n", gameSession->id);
        fprintf(file, "Players: %s vs %s\n", gameSession->players[0]->username, gameSession->players[1]->username);
        fprintf(file, "Moves played: %d\n", gameSession->numMoves);
        fprintf(file, "Current player: %s\n",
                gameSession->players[gameSession->currentPlayer]->username);
        fprintf(file, "Scores: %s=%d, %s=%d\n\n",
                gameSession->players[0]->username, gameSession->game.scores[0],
                gameSession->players[1]->username, gameSession->game.scores[1]);

        // moves history
        fprintf(file, "=== Moves history ===\n");
        for (int i = 0; i < gameSession->numMovesRecorded; i++) {
            MoveRecord *moveRecord = &gameSession->movesHistory[i];

            char tbuf[64];
            formatTime(moveRecord->t, tbuf, sizeof(tbuf));

            const char *playerName = gameSession->players[moveRecord->playerNum]->username;
            if (moveRecord->playerNum > -1 && moveRecord->house > -1) {
                fprintf(file, "Move %d | %s | player=%s | house=%d\n",
                        moveRecord->number, tbuf, playerName, moveRecord->house);
            }

            fputs(moveRecord->grid, file);
            fputc('\n', file);
        }

        if (gameSession->numMovesRecorded == 0) {
            fprintf(file, "(no moves recorded yet)\n");
        }

        fputc('\n', file);


        // chat history
        fprintf(file, "=== Game chat ===\n");
        for (int i = 0; i < gameSession->numGameMessages; i++) {
            ChatRecord *chatrecord = &gameSession->gameMessages[i];

            char tbuf[64];
            formatTime(chatrecord->t, tbuf, sizeof(tbuf));

            fprintf(file, "[%s] %s: %s\n", tbuf, chatrecord->sender, chatrecord->text);
        }

        if (gameSession->numGameMessages == 0) {
            fprintf(file, "(no chat messages)\n");
        }

        fclose(file);
    }


    // Send file to client
    char begin[2*BUF_SIZE];
    sprintf(begin, "BEGIN_SAVED_GAME %s\n", filename);
    writeClient(client->sock, begin);

    FILE *readFile = fopen(filepath, "r");
    if (!readFile) {
        writeClient(client->sock, "Error: Failed to read saved file.\nEND_SAVED_GAME\n");
        return 0;
    }
    char line[2*BUF_SIZE];
    while (fgets(line, sizeof(line), readFile)) {
        writeClient(client->sock, line);
    }
    fclose(readFile);

    writeClient(client->sock, "END_SAVED_GAME\n");

    return 1;
}
