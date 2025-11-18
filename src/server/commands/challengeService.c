#include <string.h>
#include <stdio.h>

#include "challengeService.h"


int challenge_send(Client **connectedClients, Client *challenger,
                   int actualConnected, char username[])
// Sends a challenge from the challenger to another connected user specified by username.
// Errors : challenging self, user not found, already challenged
{
    if (strcmp(challenger->username, username) == 0)
    {
        char msg[] = "Error: You cannot challenge yourself.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    Client *challenged = findConnectedClientByUsername(connectedClients, actualConnected, username);
    if (challenged == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(challenger->sock, msg);
        return 0;
    }

    int addChallengeResult = addChallenge(challenger, challenged);
    if (addChallengeResult == 0)
    {
        char msg[] = "Error: You have already challenged this user.";
        writeClient(challenger->sock, msg);
        return 0;
    }
    else if (addChallengeResult == -1)
    {
        char msg[] = "Error: You have reached the maximum number of pending challenges. Please wait for one to be accepted.";
        writeClient(challenger->sock, msg);
        return 0;
    }
    else if (addChallengeResult == -2)
    {
        char msg[] = "Error: The user you are trying to challenge has reached the maximum number of pending challenges. Please try again later.";
        writeClient(challenged->sock, msg);
        return 0;
    }

    char message[2 * BUF_SIZE];
    snprintf(message, 2 * BUF_SIZE, "CHALLENGE_FROM %s", challenger->username);
    writeClient(challenged->sock, message);

    return 1;
}

int challenge_accept(Client **connectedClients, Client *client,
                     int actualConnected, char challenger[],
                     int *numGames,
                     GameSession **activeGameSessions, int *numActiveGames)
// Accepts a challenge from another user and initiates the game setup process.
// Errors : challenger not found, already in a game, too many active games
{
    Client *challengerClient = findConnectedClientByUsername(connectedClients, actualConnected, challenger);
    if (challengerClient == NULL)
    {
        // challenger not found
        char msg[] = "Error : challenger not found\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (client->gameId != NULL)
    {
        char msg[] = "Error: You are already in a game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (challengerClient->gameId != NULL)
    {
        char msg[] = "Error: Challenger is already in a game.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (*numActiveGames > MAX_ACTIVE_GAMES - 1 || *numGames > MAX_GAMES - 1)
    {
        char msg[] = "Error: Too many active games. Please wait.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    if (removeChallenge(challengerClient, client) == 0)
    {
        char msg[] = "Error: No pending challenge from that user.";
        writeClient(client->sock, msg);
        return 0;
    }

    char message[2 * BUF_SIZE];
    snprintf(message, 2 * BUF_SIZE, "CHALLENGE_ACCEPTED_BY %s", client->username);
    writeClient(challengerClient->sock, message);

    // Ask for rotation, then start the game
    askClientInput(client->sock, "HIDDEN_STARTGAME", "1", challengerClient->username, "1", "Enter rotation (0 for counter-clockwise, 1 for clockwise): ");

    return 1;
}

int challenge_decline(Client *clients, Client *client,
                      int actualClients, char challenger[])
// Declines a challenge from another user and notifies the challenger.
// Errors : challenger not found
{
    Client *challengerClient = findClientByUsername(clients, actualClients, challenger);
    if (challengerClient == NULL)
    {
        // challenger not found
        char msg[] = "Error : challenger not found\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // Remove the pending challenge
    if (removeChallenge(challengerClient, client) == 0)
    {
        char msg[] = "Error: No pending challenge from that user.";
        writeClient(client->sock, msg);
        return 0;
    }

    char message[2 * BUF_SIZE];
    snprintf(message, 2 * BUF_SIZE, "CHALLENGE_DECLINED_BY %s", client->username);
    writeClient(challengerClient->sock, message);

    return 1;
}

int challenge_removeSent(Client *clients, Client *client,
                         int actualClient, char username[])
// Removes a specific sent challenge from the client to another user specified by username.
// Errors : user not found, no pending challenge to that user
{
    Client *challengedClient = findClientByUsername(clients, actualClient, username);
    if (challengedClient == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    if (removeChallenge(client, challengedClient) == 0)
    {
        char msg[] = "Error: No pending challenge from that user.";
        writeClient(challengedClient->sock, msg);
        return 0;
    }

    char msg[] = "Pending challenge removed.\n";
    writeClient(client->sock, msg);

    return 1;
}

void challenge_seePending(Client *client)
// Displays the list of pending challenges received by the client.
// Errors : none
{
    char message[BUF_SIZE];
    message[0] = '\0';

    if (client->numPendingChallengesFrom == 0)
    {
        strncat(message, "No pending challenges.\n", BUF_SIZE - strlen(message) - 1);
    }
    else
    {
        strncat(message, "Pending challenges from:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < client->numPendingChallengesFrom; i++)
        {
            strncat(message, client->pendingChallengesFrom[i], BUF_SIZE - strlen(message) - 1);
            strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(client->sock, message);
}

void challenge_seeSent(Client *client)
// Displays the list of challenges sent by the client.
// Errors : none
{
    char message[BUF_SIZE];
    message[0] = '\0';

    if (client->numPendingChallengesTo == 0)
    {
        strncat(message, "No sent challenges.\n", BUF_SIZE - strlen(message) - 1);
    }
    else
    {
        strncat(message, "Sent challenges to:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < client->numPendingChallengesTo; i++)
        {
            strncat(message, client->pendingChallengesTo[i], BUF_SIZE - strlen(message) - 1);
            strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(client->sock, message);
}

void challenge_clearPending(Client *client)
// Clears all pending challenges received by the client.
// Errors : none
{
    clearReceivedChallenge(client);
    char msg[] = "All received pending challenges cleared.\n";
    writeClient(client->sock, msg);
}

void challenge_clearSent(Client *client)
// Clears all challenges sent by the client.
// Errors : none
{
    clearSentChallenge(client);
    char msg[] = "All sent pending challenges cleared.\n";
    writeClient(client->sock, msg);
}
