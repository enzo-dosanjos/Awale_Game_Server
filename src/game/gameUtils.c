#include <stdlib.h>

#include "gameUtils.h"

int **initGrid(int nbPlayers, int nbHouses, int nbSeeds)
{
    int **grid = malloc(nbPlayers * sizeof(int *));
    int *data = malloc(nbPlayers * nbHouses * sizeof(int));

    for (int i = 0; i < nbPlayers; i++)
    {
        grid[i] = data + i * nbHouses;
    }

    int nbSeedsPerHouse = nbSeeds / (nbHouses * nbPlayers);

    for (int i = 0; i < nbPlayers; i++)
    {
        for (int j = 0; j < nbHouses; j++)
        {
            grid[i][j] = nbSeedsPerHouse;
        }
    }

    return grid;
}

void copyGame(Game *game, Game *copy, int nbPlayers, int nbHouses)
{
    int **grid = malloc(nbPlayers * sizeof(int *));
    int *data = malloc(nbPlayers * nbHouses * sizeof(int));

    for (int i = 0; i < nbPlayers; i++)
    {
        grid[i] = data + i * nbHouses;
    }

    for (int i = 0; i < nbPlayers; i++)
    {
        for (int j = 0; j < nbHouses; j++)
        {
            grid[i][j] = game->grid[i][j];
        }
    }

    copy->grid = grid;
    copy->rotation = game->rotation;
    copy->scores = malloc(nbPlayers * sizeof(int));
    for (int i = 0; i < nbPlayers; i++)
    {
        copy->scores[i] = game->scores[i];
    }
}

int checkFamishedPlayer(Game *game, int numPlayer, int nbHouses)
{
    int empty = 1;

    int i = 0;
    while (i < nbHouses && empty)
    {
        if (game->grid[numPlayer][i] != 0)
        {
            empty = 0;
        }

        i++;
    }

    if (empty)
        return 1;
    return 0;
}

int checkLegalMove(Game *game, Move move, int nbPlayers, int nbHouses)
{
    // Check if the house is empty
    if (game->grid[move.numPlayer][move.houseNum] == 0)
    {
        return 0;
    }

    int famishedCurr = checkFamishedPlayer(
            game, (move.numPlayer + 1) % nbPlayers, nbHouses);

    Game tempGame;
    copyGame(game, &tempGame, nbPlayers, nbHouses);
    makeAMove(&tempGame, move, 1, nbPlayers, nbHouses);
    int famishedNext = checkFamishedPlayer(
            &tempGame, (move.numPlayer + 1) % nbPlayers, nbHouses);

    // Check if the move feeds a famished opponent
    if (famishedCurr && famishedNext)
    {
        return 0;
    }

    // Check if the move would starve the opponent
    if (famishedNext)
    {
        return -1; // The move is legal, but no captures can be made
    }

    return 1;
}

void makeAMove(Game *game, Move move, int capturesOk, int nbPlayers,
               int nbHouses)
{
    int *gameGrid = &game->grid[0][0];

    int houseInd = move.numPlayer * nbHouses + move.houseNum;
    int numSeeds = game->grid[move.numPlayer][move.houseNum];

    int i;
    int skip = numSeeds / (nbHouses * nbPlayers);
    for (i = houseInd + 1; i < houseInd + 1 + numSeeds + skip; i++)
    {
        gameGrid[i % (nbHouses * nbPlayers)]++;
    }

    gameGrid[houseInd] = 0;

    if (capturesOk)
    {
        i = (i - 1) % (nbHouses * nbPlayers);

        while ((i / nbHouses != move.numPlayer) &&
               (gameGrid[i] == 2 || gameGrid[i] == 3))
        {
            game->scores[move.numPlayer] += gameGrid[i];
            gameGrid[i] = 0;
            i = (i - 1) % (nbHouses * nbPlayers);
        }
    }
}

int isGameOver(Game *game, int nbPlayers, int nbHouses)
{
    for (int i = 0; i < nbPlayers; i++)
    {
        int noLegalMoves = 1;

        if (game->scores[i] >= 25)
        {
            return 1;
        }

        Move move;
        move.numPlayer = i;
        for (int j = 0; j < nbHouses; j++)
        {
            move.houseNum = j;
            if (checkLegalMove(game, move, nbPlayers, nbHouses))
            {
                noLegalMoves = 0;
            }
        }

        if (noLegalMoves)
        {
            return 1;
        }
    }

    return 0;
}

void freeGame(Game *game)
{
    free(game->grid[0]);
    free(game->grid);
    free(game->scores);
}
