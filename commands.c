#include "commands.h"

#include <stdio.h>
#include <string.h>

void challenge(Client *clients, Client challenger, int actual, char username[]) {
    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_FROM %s", challenger.username);
    sendMessageToClient(clients, actual, username, message);
}

GameSession acceptChallenge(Client *clients, Client client, int actual, char challenger[]) {
    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "CHALLENGE_ACCEPTED_BY %s", client.username);
    sendMessageToClient(clients, actual, challenger, message);

    strcpy(message, "Enter rotation (0 for counter-clockwise, 1 for clockwise): ");
    sendMessageToClient(clients, actual, client.username, message);

    char rotationStr[1];
    read_client(client.sock, rotationStr);
    int rotation = atoi(rotationStr);

    GameSession gameSession;
    Game game = startGame(rotation);

    gameSession.game = game;
    gameSession.currentPlayer = 0;

    Client *challengerClient = findClientByUsername(clients, actual, challenger);
    gameSession.players[0] = *challengerClient;
    gameSession.players[1] = client;
    gameSession.id = rand(); // Random game session ID

    printf("%d", gameSession.id);

    return gameSession;
}

