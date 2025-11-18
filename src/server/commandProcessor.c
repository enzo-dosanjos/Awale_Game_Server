#include "commandProcessor.h"
#include "commands/chatService.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void processClientCommand(char *buffer, Client *client, Client *clients,
                          const int *actualClient, Client **connectedClients,
                          int *actualConnected, GameSession *gameSessions,
                          int *numGames, GameSession **activeGameSessions,
                          int *numActiveGames)
// Process a command from a logged-in client.
// Errors : client disconnected, unknown command, missing parameters
{
    const int c = readClient(client->sock, buffer);
    /* client disconnected */
    if (c == 0)
    {
        GameSession *gameSession = findGameSessionByClient(client, activeGameSessions, *numActiveGames);

        if (account_quit(connectedClients, actualConnected, client, activeGameSessions, numActiveGames) == -1)
        {
            game_freeGameSession(gameSessions, numGames, activeGameSessions, numActiveGames, gameSession);
        }
    }
    else
    {
        char *command = strtok(buffer, " ");
        if (strcmp(command, "CHALLENGE") == 0)
        {
            char *username = strtok(NULL, " ");
            if (username == NULL)
            {
                char msg[] = "Error: No username provided for challenge. Use: CHALLENGE <username>\n";
                writeClient(client->sock, msg);
                return;
            }

            challenge_send(connectedClients, client, *actualConnected, username);
        }
        else if (strcmp(command, "ACCEPT") == 0)
        {
            char *username = strtok(NULL, " ");
            if (username == NULL)
            {
                char msg[] = "Error: No username provided to accept challenge from. Use: ACCEPT <username>\n";
                writeClient(client->sock, msg);
                return;
            }

            challenge_accept(connectedClients, client, *actualConnected, username, numGames, activeGameSessions, numActiveGames);
        }
        else if (strcmp(command, "HIDDEN_STARTGAME") == 0)
        {
            char *rotationStr = strtok(NULL, ";");
            char *numParamsStr = strtok(NULL, ";");
            if (atoi(numParamsStr) < 1)
            {
                return;
            }
            char *username = strtok(NULL, ";");
            int rotation = atoi(rotationStr);

            game_start(client, connectedClients, *actualConnected, username, gameSessions, numGames, activeGameSessions, numActiveGames, rotation);
        }
        else if (strcmp(command, "DECLINE") == 0)
        {
            char *username = strtok(NULL, " ");
            if (username == NULL)
            {
                char msg[] = "Error: No username provided to decline challenge from. Use: DECLINE <username>\n";
                writeClient(client->sock, msg);
                return;
            }

            challenge_decline(clients, client, *actualClient, username);
        }
        else if (strcmp(command, "LIST") == 0)
        {
            general_listClients(connectedClients, *actualConnected, *client);
        }
        else if (strcmp(command, "LISTGAMES") == 0)
        {
            general_listGames(activeGameSessions, *numActiveGames, *client);
        }
        else if (strcmp(command, "SEEPENDINGREQ") == 0)
        {
            challenge_seePending(client);
        }
        else if (strcmp(command, "SEESENTREQ") == 0)
        {
            challenge_seeSent(client);
        }
        else if (strcmp(command, "CLEARPENDINGREQ") == 0)
        {
            challenge_clearPending(client);
        }
        else if (strcmp(command, "CLEARSENTREQ") == 0)
        {
            challenge_clearSent(client);
        }
        else if (strcmp(command, "REMOVESENTREQ") == 0)
        {
            char *username = strtok(NULL, " ");
            if (username == NULL)
            {
                char msg[] = "Error: No username provided to remove sent challenge to. Use: REMOVESENTREQ <username>\n";
                writeClient(client->sock, msg);
                return;
            }

            challenge_removeSent(clients, client, *actualClient, username);
        }
        else if (strcmp(command, "MOVE") == 0)
        {
            char *houseChar = strtok(NULL, " ");
            if (houseChar == NULL)
            {
                char msg[] = "Error: No house number provided for move. Use: MOVE <house_number>\n";
                writeClient(client->sock, msg);
                return;
            }

            int house = atoi(houseChar);
            game_move(client, activeGameSessions, numActiveGames, house);
        }
        else if (strcmp(command, "ENDGAME") == 0)
        {
            game_suggestEnd(client, activeGameSessions, numActiveGames);
        }
        else if (strcmp(command, "ACCEPTEND") == 0)
        {
            game_acceptEnd(client, activeGameSessions, numActiveGames);
        }
        else if (strcmp(command, "HIDDEN_HANDLEENDGAME") == 0)
        {
            char *flag = strtok(NULL, ";");
            if (flag == NULL)
            {
                char msg[] = "An error occurred while handling your request, the game could not be saved.\n";
                writeClient(client->sock, msg);
            }

            int saveFlag = (flag != NULL && (strncmp(flag, "Y", 1) == 0 || strncmp(flag, "y", 1) == 0)) ? 1 : 0;

            game_handleEndForPlayer(client, connectedClients, *actualConnected, activeGameSessions, numActiveGames, gameSessions, numGames, saveFlag);
        }
        else if (strcmp(command, "MSG") == 0)
        {
            // check if it's a private message by checking the number of tokens
            char *msgOrUsername = strtok(NULL, " ");
            if (msgOrUsername == NULL)
            {
                char msg[] = "Error: No message provided. Use: MSG <message> or MSG @<username> <message>\n";
                writeClient(client->sock, msg);
                return;
            }

            char *restOfMsg = strtok(NULL, "");
            if (msgOrUsername[0] == '@')
            {
                // It's a private message
                char *username = msgOrUsername + 1; // Skip the '@' character
                char *message = restOfMsg;
                chat_sendPrivate(connectedClients, *actualConnected, client, username, message);
            }
            else
            {
                // It's a public message
                // Reconstruct message
                char message[BUF_SIZE];
                if (restOfMsg == NULL)
                {
                    snprintf(message, BUF_SIZE, "%s\n", msgOrUsername);
                }
                else
                {
                    snprintf(message, BUF_SIZE, "%s %s\n", msgOrUsername, restOfMsg);
                }
                chat_broadcastToAll(connectedClients, *actualConnected, client, message);
            }
        }
        else if (strcmp(command, "ADDFRIEND") == 0)
        {
            char *username = strtok(NULL, " ");
            if (username == NULL)
            {
                char msg[] = "Error: No username provided to add as friend. Use: ADDFRIEND <username>\n";
                writeClient(client->sock, msg);
                return;
            }

            profile_addFriend(clients, *actualClient, client, username);
        }
        else if (strcmp(command, "REMOVEFRIEND") == 0)
        {
            char *username = strtok(NULL, " ");
            if (username == NULL)
            {
                char msg[] = "Error: No username provided to remove from friends. Use: REMOVEFRIEND <username>\n";
                writeClient(client->sock, msg);
                return;
            }

            profile_removeFriend(client, username);
        }
        else if (strcmp(command, "SHOWFRIENDS") == 0)
        {
            char *username = strtok(NULL, " ");
            // username can be NULL here to show own bio

            profile_showFriends(clients, *actualClient, client, username);
        }
        else if (strcmp(command, "WATCH") == 0)
        {
            char *gameIdStr = strtok(NULL, "");
            if (gameIdStr == NULL)
            {
                char msg[] = "Error: No game ID provided to watch. Use: WATCH <game_id>\n";
                writeClient(client->sock, msg);
                return;
            }

            int gameId = atoi(gameIdStr);
            game_watch(client, activeGameSessions, *numActiveGames, gameId);
        }
        else if (strcmp(command, "MSGGAME") == 0)
        {
            char *message = strtok(NULL, "");
            if (message == NULL)
            {
                char msg[] = "Error: No message provided for game chat. Use: MSGGAME <message>.\n";
                writeClient(client->sock, msg);
                return;
            }
            GameSession *actualGameSession = findGameSessionByViewer(activeGameSessions, *numActiveGames, client);
            chat_sendGameMessage(actualGameSession, client, message);
        }
        else if (strcmp(command, "BIO") == 0)
        {
            char *bio = strtok(NULL, "");
            if (bio == NULL)
            {
                char msg[] = "Error: No bio provided. Use: BIO <your_bio>\n";
                writeClient(client->sock, msg);
                return;
            }

            profile_updateBio(client, bio);
        }
        else if (strcmp(command, "SHOWBIO") == 0)
        {
            char *username = strtok(NULL, " ");
            // username can be NULL here to show own bio

            profile_showBio(clients, *actualClient, client, username);
        }
        else if (strcmp(command, "SHOWSTATS") == 0)
        {
            char *username = strtok(NULL, " ");
            // username can be NULL here to show own stats

            profile_showStats(clients, *actualClient, client, username);
        }
        else if (strcmp(command, "SETPRIVACY") == 0)
        {
            char *privacyStr = strtok(NULL, " ");
            if (privacyStr == NULL)
            {
                char msg[] = "Error: No privacy setting provided. Use: SETPRIVACY <true|false>\n";
                writeClient(client->sock, msg);
                return;
            }

            if (strcmp(privacyStr, "true") == 0)
            {
                profile_setPrivacy(client, 1);
            }
            else if (strcmp(privacyStr, "false") == 0)
            {
                profile_setPrivacy(client, 0);
            }
            else
            {
                char msg[] = "Error: Invalid privacy setting. Use: SETPRIVACY <true|false>\n";
                writeClient(client->sock, msg);
                return;
            }
        }
        else if (strcmp(command, "QUIT") == 0)
        {
            account_quit(connectedClients, actualConnected, client, activeGameSessions, numActiveGames);
        }
        else if (strcmp(command, "LASTGAME") == 0)
        {
            game_loadLast(connectedClients, *actualConnected, client, activeGameSessions, numActiveGames, gameSessions, numGames);
        }
        else if (strcmp(command, "SAVEGAME") == 0)
        {
            game_saveAndSend(client, activeGameSessions, *numActiveGames);
        }
        else if (strcmp(command, "HELP") == 0)
        {
            general_sendHelp(client->sock, 1);
        }
        else
        {
            char msg[] = "Error: Unknown command. Use HELP to list every known commands\n";
            writeClient(client->sock, msg);
        }
    }
}

void processLobbyCommand(char *buffer, int lobbyIndex, Client *clients,
                         int *actualClient, Client **connectedClients,
                         int *actualConnected, SOCKET *lobby, int *actualLobby,
                         GameSession *gameSessions, int *numGames,
                         GameSession **activeGameSessions, int *numActiveGames)
// Process a command from a lobby socket (not logged in).
// Errors : client disconnected, unknown command, no username or password provided
{
    const int c = readClient(lobby[lobbyIndex], buffer);
    /* client disconnected */
    if (c == 0)
    {
        closesocket(lobby[lobbyIndex]);
    }
    else
    {
        char *command = strtok(buffer, " ");
        if (strcmp(command, "MSG") == 0)
        {
            char *msg = strtok(NULL, "");
            chat_broadcastToLobby(lobby, *actualLobby, msg);
        }
        else if (strcmp(command, "LOGIN") == 0)
        {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, "");
            if (username == NULL || password == NULL)
            {
                char msg[] = "Error: No username or password provided. Use: MSG <username> <password>\n";
                writeClient(lobby[lobbyIndex], msg);
                return;
            }

            account_login(clients, actualClient, connectedClients, actualConnected,
                activeGameSessions, numActiveGames, lobby, actualLobby,
                lobbyIndex, username, password);
        }
        else if (strcmp(command, "SIGNUP") == 0)
        {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, "");
            if (username == NULL || password == NULL)
            {
                char msg[] = "Error: No username or password provided. Use: MSG <username> <password>\n";
                writeClient(lobby[lobbyIndex], msg);
                return;
            }

            account_signUp(clients, actualClient, connectedClients, actualConnected, lobby, actualLobby, lobbyIndex, username, password);
        }
        else if (strcmp(command, "HELP") == 0)
        {
            general_sendHelp(lobby[lobbyIndex], 0);
        }
        else
        {
            char msg[] = "Error: Unknown command. Use HELP to list every known commands\n";
            writeClient(lobby[lobbyIndex], msg);
        }
    }
}
