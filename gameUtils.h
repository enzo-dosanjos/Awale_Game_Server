#include <stdlib.h>

typedef struct {
    int** grid;
    int rotation;  // 0 for counter-clockwise, 1 for clockwise
    int* scores;
} Game;

typedef struct {
    int numPlayer;
    int houseNum;
} Move;

int **initGrid(int nbPlayers, int nbHouses, int nbSeeds);

void copyGame(Game game, Game *copy, int nbPlayers, int nbHouses);

int checkFamishedPlayer(Game game, int numPlayer, int nbHouses);

int checkLegalMove(Game game, Move move, int nbPlayers, int nbHouses);

void makeAMove(Game game, Move move, int capturesOk, int nbPlayers, int nbHouses);

int isGameOver(Game game, int nbPlayers, int nbHouses);

void freeGame(Game game);