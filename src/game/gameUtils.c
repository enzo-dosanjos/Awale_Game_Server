#include <stdlib.h>

#include "gameUtils.h"

void initGrid(Game *game, int numSeeds)
{
    game->grid = malloc(game->numPlayers * sizeof(int *));
    int *data = malloc(game->numPlayers * game->numHouses * sizeof(int));

    for (int i = 0; i < game->numPlayers; i++)
    {
        game->grid[i] = data + i * game->numHouses;
    }

    int nbSeedsPerHouse = numSeeds / (game->numHouses * game->numPlayers);

    for (int i = 0; i < game->numPlayers; i++)
    {
        for (int j = 0; j < game->numHouses; j++)
        {
            game->grid[i][j] = nbSeedsPerHouse;
        }
    }
}

void copyGame(Game *game, Game *copy)
{
    copy->grid = malloc(game->numPlayers * sizeof(int *));
    int *data = malloc(game->numPlayers * game->numHouses * sizeof(int));

    for (int i = 0; i < game->numPlayers; i++)
    {
        copy->grid[i] = data + i * game->numHouses;
    }

    for (int i = 0; i < game->numPlayers; i++)
    {
        for (int j = 0; j < game->numHouses; j++)
        {
            copy->grid[i][j] = game->grid[i][j];
        }
    }

    copy->numPlayers = game->numPlayers;
    copy->numHouses = game->numHouses;

    copy->rotation = game->rotation;

    copy->scores = malloc(game->numPlayers * sizeof(int));
    for (int i = 0; i < game->numPlayers; i++)
    {
        copy->scores[i] = game->scores[i];
    }
}

int checkFamishedPlayer(Game *game, int playerNum)
{
    int empty = 1;

    int i = 0;
    while (i < game->numHouses && empty)
    {
        if (game->grid[playerNum][i] != 0)
        {
            empty = 0;
        }

        i++;
    }

    if (empty)
        return 1;
    return 0;
}

int checkLegalMove(Game *game, Move move)
{
    if (move.numPlayer < 0 || move.numPlayer >= game->numPlayers ||
        move.houseNum < 0 || move.houseNum >= game->numHouses)
    {
        return 0;
    }

    // Check if the house is empty
    if (game->grid[move.numPlayer][move.houseNum] == 0)
    {
        return 0;
    }

    int famishedCurr = checkFamishedPlayer(game, (move.numPlayer + 1) % game->numPlayers);

    Game tempGame;
    copyGame(game, &tempGame);
    makeAMove(&tempGame, move, 1);
    int famishedNext = checkFamishedPlayer(
            &tempGame, (move.numPlayer + 1) % game->numPlayers);

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

    freeGame(&tempGame);

    return 1;
}

void makeAMove(Game *game, Move move, int capturesOk)
{
    int *gameGrid = &game->grid[0][0];

    int houseInd = move.numPlayer * game->numHouses + move.houseNum;
    int numSeeds = game->grid[move.numPlayer][move.houseNum];

    int i;
    int skip = numSeeds / (game->numHouses * game->numPlayers);
    for (i = houseInd + 1; i < houseInd + 1 + numSeeds + skip; i++)
    {
        gameGrid[i % (game->numHouses * game->numPlayers)]++;
    }

    gameGrid[houseInd] = 0;

    if (capturesOk)
    {
        i = (i - 1) % (game->numHouses * game->numPlayers);

        while ((i / game->numHouses != move.numPlayer) &&
               (gameGrid[i] == 2 || gameGrid[i] == 3))
        {
            game->scores[move.numPlayer] += gameGrid[i];
            gameGrid[i] = 0;
            i = (i - 1  + game->numHouses * game->numPlayers) % (game->numHouses * game->numPlayers);
        }
    }
}

int isGameOver(Game *game)
{
    for (int i = 0; i < game->numPlayers; i++)
    {
        int noLegalMoves = 1;

        if (game->scores[i] >= 25)
        {
            return 1;
        }

        Move move;
        move.numPlayer = i;
        for (int j = 0; j < game->numHouses; j++)
        {
            move.houseNum = j;
            if (checkLegalMove(game, move))
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
