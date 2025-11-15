#include "gameLogic.h"

#include <stdio.h>
#include <stdlib.h>

Game startGame(int rotation, int numPlayers, int numHouses, int numSeeds)
{
    Game game;
    game.numPlayers = numPlayers;
    game.numHouses = numHouses;

    initGrid(&game, numSeeds);

    game.rotation = rotation;

    game.scores = malloc(numPlayers * sizeof(int));
    for (int i = 0; i < numPlayers; i++)
    {
        game.scores[i] = 0;
    }

    return game;
}

int playMove(Game *game, Move move)
{
    if (game->rotation == 0 && move.numPlayer == 0)
    {
        move.houseNum = (game->numHouses - 1) - move.houseNum;
    }
    else if (game->rotation == 1 && move.numPlayer == 1)
    {
        move.houseNum = (game->numHouses - 1) - move.houseNum;
    }

    int legalMove = checkLegalMove(game, move);
    if (!legalMove)
    {
        return 0;
    }

    makeAMove(game, move, legalMove == -1 ? 0 : 1);

    return 1;
}

int nextPlayer(int currentPlayer, Game *game) { return (currentPlayer + 1) % game->numPlayers; }

int playerSelector(Game *game) { return rand() % game->numPlayers; }

int endGame(Game *game)
{
    // Collect remaining seeds
    for (int i = 0; i < game->numPlayers; i++)
    {
        for (int j = 0; j < game->numHouses; j++)
        {
            game->scores[i] += game->grid[i][j];
            game->grid[i][j] = 0;
        }
    }

    // Determine winner
    int maxScore = -1;
    int winner = -1;
    int draw = 0;
    for (int i = 0; i < game->numPlayers; i++)
    {
        if (game->scores[i] > maxScore)
        {
            maxScore = game->scores[i];
            winner = i;
            draw = 0;
        }
        else if (game->scores[i] == maxScore)
        {
            draw = 1;
        }
    }

    return draw ? -1 : winner;
}