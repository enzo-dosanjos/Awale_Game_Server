#include "gameLogic.h"

#include <stdio.h>

Game startGame (int rotation) {
    Game game;
    game.grid = initGrid(NUM_PLAYERS, NUM_HOUSES, NUM_SEEDS);
    game.rotation = rotation;
    game.scores = malloc(NUM_PLAYERS * sizeof(int));
    for (int i = 0; i < NUM_PLAYERS; i++) {
        game.scores[i] = 0;
    }

    return game;
}

int play (Game game, Move move) {
    if (game.rotation == 0 && move.numPlayer == 0) {
        move.houseNum = (NUM_HOUSES - 1) - move.houseNum;
    } else if (game.rotation == 1 && move.numPlayer == 1) {
        move.houseNum = (NUM_HOUSES - 1) - move.houseNum;
    }

    int legalMove = checkLegalMove(game, move, NUM_PLAYERS, NUM_HOUSES);
    if (!legalMove) {
        return 0;
    }

    makeAMove(game, move, legalMove == -1 ? 0 : 1, NUM_PLAYERS, NUM_HOUSES);

    return 1;
}

int nextPlayer (int currentPlayer) {
    return (currentPlayer + 1) % NUM_PLAYERS;
}

int endGame (Game game) {
    // Collect remaining seeds
    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < NUM_HOUSES; j++) {
            game.scores[i] += game.grid[i][j];
            game.grid[i][j] = 0;
        }
    }

    // Determine winner
    int maxScore = -1;
    int winner = -1;
    int draw = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (game.scores[i] > maxScore) {
            maxScore = game.scores[i];
            winner = i;
            draw = 0;
        } else if (game.scores[i] == maxScore) {
            draw = 1;
        }
    }

    return draw ? -1 : winner;
}

int main() {
    int rotation = 0;  // 0 for counter-clockwise, 1 for clockwise
    printf("Enter rotation (0 for counter-clockwise, 1 for clockwise): ");
    scanf("%d", &(rotation));
    int currentPlayer = 0;
    Game game = startGame(rotation);

    while (isGameOver(game, NUM_PLAYERS, NUM_HOUSES)) {
        Move move;
        move.numPlayer = currentPlayer;

        printf("Player %d, enter your move (house number 0-%d): ", currentPlayer, NUM_HOUSES - 1);
        scanf("%d", &(move.houseNum));

        while (!play(game, move)) {
            printf("Illegal move. Player %d, enter your move (house number 0-%d): ", currentPlayer, NUM_HOUSES - 1);
            scanf("%d", &(move.houseNum));
        }

        printGrid(game, NUM_HOUSES, NUM_PLAYERS);

        currentPlayer = nextPlayer(currentPlayer);
    }

    int winner = endGame(game);
    printGameEnd(game, NUM_PLAYERS, winner);

    freeGame(game);

    return 0;
}
