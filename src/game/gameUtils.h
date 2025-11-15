#ifndef AWALE_GAME_SERVER_GAMEUTILS_H
#define AWALE_GAME_SERVER_GAMEUTILS_H

typedef struct
{
    int **grid;
    int rotation; // 0 for counter-clockwise, 1 for clockwise
    int *scores;
    int numPlayers;
    int numHouses;
} Game;

typedef struct
{
    int numPlayer;
    int houseNum;
} Move;

void initGrid(Game *game, int numSeeds);

void copyGame(Game *game, Game *copy);

int checkFamishedPlayer(Game *game, int playerNum);

int checkLegalMove(Game *game, Move move);

void makeAMove(Game *game, Move move, int capturesOk);

int isGameOver(Game *game);

void freeGame(Game *game);

#endif // AWALE_GAME_SERVER_GAMEUTILS_H