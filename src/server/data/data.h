#ifndef AWALE_GAME_SERVER_DATA_H
#define AWALE_GAME_SERVER_DATA_H
#include <time.h>

#include "../../constants.h"
#include "../../game/gameUtils.h"


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


typedef struct
{
    int gamesPlayed;
    int gamesWon;
    int gamesLost;
    int gamesDrawn;
    double averageMovesToWin;
    int totalSeedsCollected;
} Stats;

typedef struct
{
    int number; // move number
    int playerNum;
    int house;
    time_t t;
    char grid[BUF_SIZE]; // grid after the move
} MoveRecord;

typedef struct
{
    char sender[BUF_SIZE];
    char text[BUF_SIZE];
    time_t t;
} ChatRecord;

typedef struct
{
    SOCKET sock;
    char username[BUF_SIZE];
    char password[BUF_SIZE];
    char bio[BUF_SIZE];
    int private;
    int *gameId;
    // friends list
    int numFriends;
    char friends[MAX_FRIENDS][BUF_SIZE];
    // sent challenges to
    int numPendingChallengesTo;
    char pendingChallengesTo[MAX_PENDING_CHALLENGES][BUF_SIZE];
    // received challenges from
    int numPendingChallengesFrom;
    char pendingChallengesFrom[MAX_PENDING_CHALLENGES][BUF_SIZE];
    // stats
    Stats stats;
} Client;

typedef struct
{
    int id;
    Client *players[NUM_PLAYERS];
    Game game;
    int currentPlayer;
    int numMoves;
    // endgame
    int endGameSuggested;
    int saveAnswered;
    // viewers
    int numViewers;
    Client *viewers[MAX_VIEWERS];
    // game history
    MoveRecord movesHistory[MAX_MOVES_HISTORY];
    int numMovesRecorded;
    ChatRecord gameMessages[MAX_MESSAGES_HISTORY];
    int numGameMessages;

} GameSession;

#endif //AWALE_GAME_SERVER_DATA_H
