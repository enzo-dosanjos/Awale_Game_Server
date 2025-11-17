#include <stdio.h>
#include <string.h>

#include "generalService.h"


void general_listClients(Client **connectedClients,
                         int actualConnected, Client requester)
// Lists all connected clients along with their game status.
// Errors : none
{
    char message[BUF_SIZE];
    message[0] = '\0';
    strncat(message, "Connected users:\n", BUF_SIZE - strlen(message) - 1);

    // Find the maximum username length for formatting
    int maxLen = 0;
    for (int j = 0; j < actualConnected; j++)
    {
        int currLen = (int)strlen(connectedClients[j]->username);
        if (currLen > maxLen)
        {
            maxLen = currLen;
        }
    }

    for (int i = 0; i < actualConnected; i++)
    {
        strncat(message, connectedClients[i]->username, BUF_SIZE - strlen(message) - 1);

        int pad = maxLen - (int)strlen(connectedClients[i]->username) + 1;
        if (pad < 1)
        {
            pad = 1;
        }

        char spaces[BUF_SIZE];
        memset(spaces, ' ', (size_t)pad);
        spaces[pad] = '\0';
        strncat(message, spaces, BUF_SIZE - strlen(message) - 1);

        if (connectedClients[i]->gameId != NULL)
        {
            strncat(message, "in game", BUF_SIZE - strlen(message) - 1);
        }

        strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
    }
    writeClient(requester.sock, message);
}

void general_listGames(GameSession **gameSessions,
                       int actualGame, Client requester)
// Lists all ongoing game sessions along with player information.
// Errors : none
{
    char message[BUF_SIZE];
    message[0] = '\0';

    if (actualGame == 0)
    {
        strncat(message, "No ongoing games.\n", BUF_SIZE - strlen(message) - 1);
    }
    else
    {
        strncat(message, "Ongoing games:\n", BUF_SIZE - strlen(message) - 1);
        for (int i = 0; i < actualGame; i++)
        {
            char gameInfo[3 * BUF_SIZE];
            snprintf(gameInfo, 3 * BUF_SIZE, "Game ID: %d | Players: %s vs %s\n",
                     gameSessions[i]->id,
                     gameSessions[i]->players[0]->username,
                     gameSessions[i]->players[1]->username);
            strncat(message, gameInfo, BUF_SIZE - strlen(message) - 1);
        }
    }
    writeClient(requester.sock, message);
}

void general_sendHelp(SOCKET sock, int loggedIn)
// Sends a help message to the client, listing available commands based on login status.
// Errors : none
{
    char out[8 * BUF_SIZE];
    out[0] = '\0';

    strcat(out, "Available commands:");
    strcat(out, "\n");

    if (!loggedIn)
    {
        strcat(out, "\n");
        strcat(out, "Lobby:\n");
        strcat(out, "  HELP                                   - Show this help.\n");
        strcat(out, "  MSG <message>                          - Send a message to the lobby.\n");
        strcat(out, "  LOGIN <username> <password>            - Log into an existing account.\n");
        strcat(out, "  SIGNUP <username> <password>           - Create a new account and connect.\n");
    }
    else
    {
        strcat(out, "\n");
        strcat(out, "General:\n");
        strcat(out, "  HELP                                   - Show this help.\n");
        strcat(out, "  LIST                                   - List connected users and if they are currently playing.\n");
        strcat(out, "  LISTGAMES                              - List ongoing games.\n");
        strcat(out, "  QUIT                                   - Disconnect.\n");

        strcat(out, "\n");
        strcat(out, "Challenges:\n");
        strcat(out, "  CHALLENGE <username>                   - Send a game challenge.\n");
        strcat(out, "  ACCEPT <username>                      - Accept a pending challenge.\n");
        strcat(out, "  DECLINE <username>                     - Decline a pending challenge.\n");
        strcat(out, "  SEEPENDINGREQ                          - List received challenges.\n");
        strcat(out, "  SEESENTREQ                             - List sent challenges.\n");
        strcat(out, "  CLEARPENDINGREQ                        - Clear received challenges.\n");
        strcat(out, "  CLEARSENTREQ                           - Clear sent challenges.\n");
        strcat(out, "  REMOVESENTREQ <username>               - Unsend a challenge.\n");

        strcat(out, "\n");
        strcat(out, "Messaging:\n");
        strcat(out, "  MSG <message>                          - Send a message to the general chat.\n");
        strcat(out, "  MSG @<username> <message>              - Send a private message.\n");

        strcat(out, "\n");
        strcat(out, "Game:\n");
        strcat(out, "  MSGGAME <message>                      - Send a message to the current game chat.\n");
        strcat(out, "  MOVE <house>                           - Play a move when it's your turn.\n");
        strcat(out, "  ENDGAME                                - Propose to end the current game.\n");
        strcat(out, "  ACCEPTEND                              - Accept the endgame proposal.\n");
        strcat(out, "  LASTGAME                               - Load your last unfinished game.\n");
        strcat(out, "  SAVEGAME                               - Save the current game.\n");

        strcat(out, "\n");
        strcat(out, "Spectating:\n");
        strcat(out, "  LISTGAMES                              - List ongoing games ids.\n");
        strcat(out, "  WATCH <gameId>                         - Watch a game (respects privacy).\n");
        strcat(out, "  MSGGAME <message>                      - Send a message to the watched game chat.\n");

        strcat(out, "\n");
        strcat(out, "Profile:\n");
        strcat(out, "  BIO <text>                             - Update your bio.\n");
        strcat(out, "  SHOWBIO [username]                     - Show your or someone's bio (respects privacy).\n");
        strcat(out, "  SHOWSTATS [username]                   - Show your or someone's stats (respects privacy).\n");
        strcat(out, "  ADDFRIEND <username>                   - Add a friend.\n");
        strcat(out, "  REMOVEFRIEND <username>                - Remove a friend.\n");
        strcat(out, "  SHOWFRIENDS [username]                 - Show your or someone's friends list (respects privacy).\n");
        strcat(out, "  SETPRIVACY <true|false>                - true makes your bio/game private to friends.\n");
    }

    writeClient(sock, out);
}