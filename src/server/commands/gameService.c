#include <string.h>
#include <stdio.h>

// For saveGameAndSend
#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(path) _mkdir(path)
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #define MKDIR(path) mkdir(path, 0755)
#endif

#include "gameService.h"


int game_start(Client *client, Client **connectedClients,
               int actualConnected, char challenger[],
               GameSession *gameSessions, int *numGames,
               GameSession **activeGameSessions, int *numActiveGames,
               int rotation)
// Handles the actual start of the game after a challenge has been accepted and the rotation has been specified.
// Errors : challenger not found, too many active games
{
    Client *challengerClient = findConnectedClientByUsername(connectedClients, actualConnected, challenger);
    if (challengerClient == NULL)
    {
        return 0;
    }

    Game game = startGame(rotation, NUM_PLAYERS, NUM_HOUSES, NUM_SEEDS);
    int firstPlayer = playerSelector(&game);
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

int game_move(Client *client,
              GameSession **activeGameSessions, int *numActiveGames,
              int house)
// Processes a move made by the client in their current game session.
// Errors : not in a game, not client's turn, illegal move
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

    int next = nextPlayer(gameSession->currentPlayer, &gameSession->game);
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

    if (isGameOver(&gameSession->game))
    {
        // Ask the players if they want to save the game, then handle endgame
        char saveMsg[] = "CLIENT_INPUT HIDDEN_HANDLEENDGAME N _ The game has ended. Do you want to save the game? (Y/N): ";
        for (int i = 0; i < NUM_PLAYERS; i++)
        {
            writeClient(gameSession->players[i]->sock, saveMsg);
        }
    }

    return 1;
}

int game_suggestEnd(Client *client,
                    GameSession **activeGameSessions, int *numActiveGames)
// Suggests ending the current game session for the client.
// Errors : not in a game
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
            // Ask the players if they want to save the game, then handle endgame
            char saveMsg[] = "CLIENT_INPUT HIDDEN_HANDLEENDGAME N _ The game has ended. Do you want to save the game? (Y/N): ";
            for (int i = 0; i < NUM_PLAYERS; i++)
            {
                writeClient(gameSession->players[i]->sock, saveMsg);
            }
        }
        gameSession->endGameSuggested = 0;
    }
    else if (gameSession->players[1] == client)
    {
        if (gameSession->endGameSuggested == 0)
        {
            // Ask the players if they want to save the game, then handle endgame
            char saveMsg[] = "CLIENT_INPUT HIDDEN_HANDLEENDGAME N _ The game has ended. Do you want to save the game? (Y/N): ";
            for (int i = 0; i < NUM_PLAYERS; i++)
            {
                writeClient(gameSession->players[i]->sock, saveMsg);
            }
        }
        gameSession->endGameSuggested = 1;
    }

    Client *opponent = gameSession->players[nextPlayer(gameSession->endGameSuggested, &gameSession->game)];
    writeClient(opponent->sock, "The opponent suggests ending this game. ACCEPTEND?\n");

    return 1;
}

int game_acceptEnd(Client *client,
                   GameSession **activeGameSessions, int *numActiveGames)
// Accepts the suggestion to end the current game session for the client.
// Errors : not in a game
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
        // Ask the players if they want to save the game, then handle endgame
        char saveMsg[] = "CLIENT_INPUT HIDDEN_HANDLEENDGAME N _ The game has ended. Do you want to save the game? (Y/N): ";
        for (int i = 0; i < NUM_PLAYERS; i++)
        {
            writeClient(gameSession->players[i]->sock, saveMsg);
        }

        return 1;
    }

    return 0;
}

void game_handleEndForPlayer(Client *client,
                             Client **connectedClients, int actualConnected,
                             GameSession **activeGameSessions,
                             int *numActiveGames,
                             GameSession *gameSessions, int *numGames,
                             int saveFlag)
// Handles the endgame process for a player, including saving the game if requested and updating player statistics.
// Errors : none
{
    GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);
    int winner = endGame(&gameSession->game);

    if (saveFlag)
    {
        game_saveAndSend(client, activeGameSessions, *numActiveGames);
    }

    char message[BUF_SIZE] = "\0";
    char usernames[NUM_PLAYERS][BUF_SIZE];
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        strcpy(usernames[i], gameSession->players[i]->username);
    }
    printGameEndMessage(message, &gameSession->game, NUM_PLAYERS, winner, usernames);
    writeClient(client->sock, message);

    client->gameId = NULL;
    client->stats.gamesPlayed++;

    if (winner == -1)
    {
        client->stats.gamesDrawn++;
    }
    else if (client == gameSession->players[winner])
    {
        double prevAvg = client->stats.averageMovesToWin;
        client->stats.averageMovesToWin = (prevAvg * client->stats.gamesWon + gameSession->numMoves / NUM_PLAYERS) / (client->stats.gamesWon + 1);
        client->stats.gamesWon++;
    }
    else
    {
        client->stats.gamesLost++;
    }

    int i = findClientIndex(connectedClients, actualConnected, client);
    client->stats.totalSeedsCollected += gameSession->game.scores[i];

    gameSession->saveAnswered++;

    if (gameSession->saveAnswered == NUM_PLAYERS) {
        removeActiveGameSession(activeGameSessions, numActiveGames, gameSession->id);
        removeGameSession(gameSessions, numGames, gameSession->id);
        freeGame(&gameSession->game);
    }
}

int game_loadLast(Client **connectedClients, int actualConnected,
                  Client *client,
                  GameSession **activeGameSessions, int *numActiveGames,
                  GameSession *gameSessions, int *numGames)
// Loads the last saved game for the specified client.
// Errors : no saved games, already in a game, not a player in any saved game, opponent not connected or already in a game
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

int game_saveAndSend(Client *client, GameSession **activeGameSessions,
                     int numActiveGames)
// Saves the current game session for the specified client and sends the saved game file to the client.
// Errors : not in a game, file save error
{
if (!client->gameId) {
        writeClient(client->sock, "Error: You are not currently in a game.\n");
        return 0;
    }

    GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, numActiveGames);
    if (!gameSession) {
        writeClient(client->sock, "Error: Could not find your game session.\n");
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

// Spectating
int game_watch(Client *client, GameSession **gameSessions, int actualGame,
                    int gameId)
// Allows a client to watch an ongoing game session specified by gameId.
// Errors : already in a game, game not found, max viewers reached, private game access denied
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
