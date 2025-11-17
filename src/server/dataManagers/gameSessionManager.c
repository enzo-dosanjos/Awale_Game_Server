#include <string.h>
#include <time.h>


#include "gameSessionManager.h"



GameSession *initGameSession(GameSession *gameSessions, int *numGames,
                             Game *game, int firstPlayer, Client *player1,
                             Client *player2)
// Initializes a new game session with the provided game state and players.
// Returns a pointer to the newly created game session.
// Errors : too many games
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

int removeGameSession(GameSession *gameSessions, int *numGames, int gameId)
// Removes a game session with the specified game ID from the list of game sessions.
// Errors : not found
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

int removeActiveGameSession(GameSession **activeGameSessions, int *numGames,
                            int gameId)
// Removes an active game session with the specified game ID from the list of active game sessions.
// Errors : not found
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

void recordMove(GameSession *gameSession, const Move *move, const char *grid)
// Records a move made in the game session along with the resulting grid state.
// Errors : moves history full
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
// Records a chat message sent in the game session.
// Errors : messages history full
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

GameSession *findGameSessionByClient(Client *client, GameSession **gameSessions,
                                     int actualGame)
// Finds the game session that the given client is a player in.
// Returns a pointer to the game session if found, otherwise NULL.
// Errors : not found
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

GameSession *findGameSessionByViewer(GameSession **gameSessions, int actualGame,
                                     Client *viewer)
// Finds the game session that the given client is viewing.
// Returns a pointer to the game session if found, otherwise NULL.
// Errors : not found
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
