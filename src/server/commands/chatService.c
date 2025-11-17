#include <stdio.h>

#include "chatService.h"


void chat_broadcastToAll(Client **connectedClients, int actualConnected,
                         Client *sender, const char *message)
// Sends a chat message from the sender to all connected users.
// Errors : none
{
    sendMessageToAllClients(connectedClients, actualConnected, sender->username, message, 0);
}

void chat_broadcastToLobby(SOCKET *lobby, int actualLobby, const char *message)
// Sends a chat message to all users in the lobby (not logged in).
// Errors : none
{
    sendMessageToLobby(lobby, actualLobby, message);
}

void chat_sendPrivate(Client **connectedClients, int actualConnected,
                      Client *sender, char *targetUsername,
                      const char *message)
// Sends a private message from the sender to another connected user specified by username.
// Errors : user not found
{
    Client *client = findConnectedClientByUsername(connectedClients, actualConnected, targetUsername);
    if (client == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(sender->sock, msg);
        return;
    }

    // Format message to add sender's name
    char formattedMessage[2 * BUF_SIZE];
    snprintf(formattedMessage, 2 * BUF_SIZE, "%s (private): %s\n", sender->username, message);

    writeClient(client->sock, formattedMessage);
}

int chat_sendGameMessage(GameSession *gameSession, Client *sender,
                         const char *message)
// Sends a chat message from the sender to all participants (players and viewers) in the specified game session.
// Errors : sender not part of the game
{
    if (gameSession == NULL)
    {
        writeClient(sender->sock, "Error: You are not watching or playing any game.\n");
        return 0;
    }

    int found = 0;
    for (int i = 0; i < gameSession->numViewers; i++)
    {
        Client *viewer = gameSession->viewers[i];
        if (viewer->username == sender->username)
        {
            found = 1;
            break;
        }
    }

    if (!found && sender->gameId != NULL && *(sender->gameId) != gameSession->id)
    {
        char msg[] = "Error: You are not part of this game.\n";
        writeClient(sender->sock, msg);
        return 0;
    }

    // Format message to add sender's name
    char formattedMessage[2 * BUF_SIZE];
    snprintf(formattedMessage, 2 * BUF_SIZE, "%s (game chat): %s\n", sender->username, message);

    // Send to viewers
    for (int i = 0; i < gameSession->numViewers; i++)
    {
        Client *recipient = gameSession->viewers[i];
        writeClient(recipient->sock, formattedMessage);
    }

    // Send to players
    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        Client *player = gameSession->players[i];
        writeClient(player->sock, formattedMessage);
    }

    // Log the message in game history
    recordChat(gameSession, sender->username, message);

    return 1;
}
