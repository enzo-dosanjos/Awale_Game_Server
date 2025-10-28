#include "gameLogic.h"

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
    if (game.rotation == 0) {
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
    int rotation = 1;  // 0 for clockwise, 1 for counter-clockwise
    Game game = startGame(rotation);

    // while (not isGameOver(game)) {

    Move move;
    move.numPlayer = 1;
    move.houseNum = 5;

    play(game, move);

    printGrid(game, NUM_HOUSES, NUM_PLAYERS);

    int winner = endGame(game);
    printGameEnd(game, NUM_PLAYERS, winner);

    freeGame(game);

    return 0;
}
