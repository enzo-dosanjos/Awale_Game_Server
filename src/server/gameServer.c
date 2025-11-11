#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gameServer.h"

void initServer(void)
{
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err < 0)
    {
        puts("WSAStartup failed !");
        exit(EXIT_FAILURE);
    }
#endif
}

void endServer(void)
{
#ifdef WIN32
    WSACleanup();
#endif
}

void appServer(void)
{
    SOCKET sock = initConnectionServer();
    char buffer[BUF_SIZE];
    /* the index for the array */
    int actualClient = 0;
    int actualConnected = 0;
    int actualLobby = 0;
    int max = sock;
    int numGames = 0;
    int numActiveGames = 0;
    /* an array for all clients */
    SOCKET lobby[MAX_CONNECTED_CLIENTS];
    Client clients[MAX_CLIENTS];
    Client *connectedClients[MAX_CONNECTED_CLIENTS];
    GameSession gameSessions[MAX_GAMES];
    GameSession *activeGameSessions[MAX_ACTIVE_GAMES];

    fd_set rdfs;

    while (1)
    {
        int i = 0;
        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* add the connection socket */
        FD_SET(sock, &rdfs);

        /* add socket of each player in the lobby */
        for (i = 0; i < actualLobby; i++)
        {
            FD_SET(lobby[i], &rdfs);
        }

        /* add socket of each connected player */
        for (i = 0; i < actualConnected; i++)
        {
            FD_SET(connectedClients[i]->sock, &rdfs);
        }

        if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if (FD_ISSET(STDIN_FILENO, &rdfs))
        {
            /* stop process when type on keyboard */
            break;
        }
        else if (FD_ISSET(sock, &rdfs))
        {
            /* new client */
            SOCKADDR_IN csin = {0};
            socklen_t sinsize = sizeof csin;
            int csock = accept(sock, (SOCKADDR *) &csin, &sinsize);
            if (csock == SOCKET_ERROR)
            {
                perror("accept()");
                continue;
            }

            max = csock > max ? csock : max;
            FD_SET(csock, &rdfs);

            lobby[actualLobby] = csock;
            actualLobby++;

            char msg[] = "Bienvenue dans le lobby !\n";
            writeClient(csock, msg);
        }
        else
        {
            int i = 0;
            for (i = 0; i < actualConnected; i++)
            {
                /* a client is talking */
                if (FD_ISSET(connectedClients[i]->sock, &rdfs))
                {
                    Client *client = connectedClients[i];
                    int c = readClient(client->sock, buffer);
                    /* client disconnected */
                    if (c == 0)
                    {
                        quit(connectedClients, &actualConnected, client,
                             activeGameSessions, &numActiveGames, gameSessions,
                             &numGames);
                    }
                    else
                    {
                        char *command = strtok(buffer, " ");
                        if (strcmp(command, "CHALLENGE") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            if (username == NULL)
                            {
                                char msg[] =
                                        "Error: No username provided for "
                                        "challenge. Use: CHALLENGE "
                                        "<username>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            challenge(connectedClients, client, actualConnected,
                                      username);
                        }
                        else if (strcmp(command, "ACCEPT") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            if (username == NULL)
                            {
                                char msg[] =
                                        "Error: No username provided to accept "
                                        "challenge from. Use: ACCEPT "
                                        "<username>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            acceptChallenge(
                                    connectedClients, client, actualConnected,
                                    username, gameSessions, &numGames,
                                    activeGameSessions, &numActiveGames);
                        }
                        else if (strcmp(command, "DECLINE") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            if (username == NULL)
                            {
                                char msg[] =
                                        "Error: No username provided to "
                                        "decline challenge from. Use: DECLINE "
                                        "<username>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            declineChallenge(connectedClients, client,
                                             actualConnected, username);
                        }
                        else if (strcmp(command, "LIST") == 0)
                        {
                            listClients(connectedClients, actualConnected,
                                        *client);
                        }
                        else if (strcmp(command, "LISTGAMES") == 0)
                        {
                            listGames(activeGameSessions, numActiveGames,
                                      *client);
                        }
                        else if (strcmp(command, "SEEPENDINGREQ") == 0)
                        {
                            seePendingReq(client);
                        }
                        else if (strcmp(command, "SEESENTREQ") == 0)
                        {
                            seeSentReq(client);
                        }
                        else if (strcmp(command, "CLEARPENDINGREQ") == 0)
                        {
                            clearPendingReq(client);
                        }
                        else if (strcmp(command, "CLEARSENTREQ") == 0)
                        {
                            clearSentReq(client);
                        }
                        else if (strcmp(command, "REMOVESENTREQ") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            if (username == NULL)
                            {
                                char msg[] =
                                        "Error: No username provided to remove "
                                        "sent challenge to. Use: REMOVESENTREQ "
                                        "<username>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            removeSentReq(connectedClients, client,
                                          actualConnected, username);
                        }
                        else if (strcmp(command, "MOVE") == 0)
                        {
                            char *houseChar = strtok(NULL, " ");
                            if (houseChar == NULL)
                            {
                                char msg[] =
                                        "Error: No house number provided for "
                                        "move. Use: MOVE <house_number>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            int house = atoi(houseChar);
                            move(client, activeGameSessions, &numActiveGames,
                                 gameSessions, &numGames, house);
                        }
                        else if (strcmp(command, "ENDGAME") == 0)
                        {
                            suggestEndgame(client, activeGameSessions,
                                           &numActiveGames, gameSessions,
                                           &numGames);
                        }
                        else if (strcmp(command, "ACCEPTEND") == 0)
                        {
                            acceptEndgame(client, activeGameSessions,
                                          &numActiveGames, gameSessions,
                                          &numGames);
                        }
                        else if (strcmp(command, "MSG") == 0)
                        {
                            // check if it's a private message by checking the
                            // number of tokens
                            char *msgOrUsername = strtok(NULL, " ");
                            if (msgOrUsername == NULL)
                            {
                                char msg[] =
                                        "Error: No message provided. Use: MSG "
                                        "<message> or MSG @<username> "
                                        "<message>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            char *restOfMsg = strtok(NULL, "");
                            if (msgOrUsername[0] == '@')
                            {
                                // It's a private message
                                char *username = msgOrUsername +
                                                 1; // Skip the '@' character
                                char *message = restOfMsg;
                                sendMP(connectedClients, client,
                                       actualConnected, username, message);
                            }
                            else
                            {
                                // It's a public message
                                // Reconstruct message
                                char message[BUF_SIZE];
                                if (restOfMsg == NULL)
                                {
                                    snprintf(message, BUF_SIZE, "%s\n",
                                             msgOrUsername);
                                }
                                else
                                {
                                    snprintf(message, BUF_SIZE, "%s %s\n",
                                             msgOrUsername, restOfMsg);
                                }
                                sendMessageToAllClients(
                                        connectedClients, client->username,
                                        actualConnected, message, 0);
                            }
                        }
                        else if (strcmp(command, "ADDFRIEND") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            if (username == NULL)
                            {
                                char msg[] =
                                        "Error: No username provided to add as "
                                        "friend. Use: ADDFRIEND <username>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            addFriend(client, username);
                        }
                        else if (strcmp(command, "WATCH") == 0)
                        {
                            char *gameIdStr = strtok(NULL, "");
                            if (gameIdStr == NULL)
                            {
                                char msg[] =
                                        "Error: No game ID provided to watch. "
                                        "Use: WATCH <game_id>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            int gameId = atoi(gameIdStr);
                            watchGame(client, activeGameSessions,
                                      numActiveGames, gameId);
                        }
                        else if (strcmp(command, "MSGGAME") == 0)
                        {
                            char *message = strtok(NULL, "");
                            if (message == NULL)
                            {
                                char msg[] =
                                        "Error: No message provided for game "
                                        "chat. Use: MSGGAME <message>.\n";
                                writeClient(client->sock, msg);
                                continue;
                            }
                            GameSession *actualGameSession =
                                    findGameSessionByViewer(activeGameSessions,
                                                            numActiveGames,
                                                            client);
                            SendMsgGame(actualGameSession, client, message);
                        }
                        else if (strcmp(command, "BIO") == 0)
                        {
                            char *bio = strtok(NULL, "");
                            if (bio == NULL)
                            {
                                char msg[] =
                                        "Error: No bio provided. Use: BIO "
                                        "<your_bio>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            updateBio(client, bio);
                        }
                        else if (strcmp(command, "SHOWBIO") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            // username can be NULL here to show own bio

                            showBio(connectedClients, actualConnected, client,
                                    username);
                        }
                        else if (strcmp(command, "SHOWSTATS") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            // username can be NULL here to show own stats

                            showStats(connectedClients, actualConnected, client,
                                      username);
                        }
                        else if (strcmp(command, "SETPRIVACY") == 0)
                        {
                            char *privacyStr = strtok(NULL, " ");
                            if (privacyStr == NULL)
                            {
                                char msg[] =
                                        "Error: No privacy setting provided. "
                                        "Use: SETPRIVACY <true|false>\n";
                                writeClient(client->sock, msg);
                                continue;
                            }

                            if (strcmp(privacyStr, "true") == 0)
                            {
                                setPrivacy(client, 1);
                            }
                            else
                            {
                                setPrivacy(client, 0);
                            }
                        }
                        else if (strcmp(command, "QUIT") == 0)
                        {
                            quit(connectedClients, &actualConnected, client,
                                 activeGameSessions, &numActiveGames,
                                 gameSessions, &numGames);
                        }
                        else if (strcmp(command, "LASTGAME") == 0)
                        {
                            loadGame(connectedClients, actualConnected, client,
                                     activeGameSessions, &numActiveGames,
                                     gameSessions, &numGames);
                        }
                        else if (strcmp(command, "HELP") == 0)
                        {
                            sendHelp(client->sock, 1);
                        }
                        else
                        {
                            char msg[] =
                                    "Error: Unknown command. Use HELP to list "
                                    "every known commands\n";
                            writeClient(client->sock, msg);
                        }
                    }
                    break;
                }
            }

            for (i = 0; i < actualLobby; i++)
            {
                if (FD_ISSET(lobby[i], &rdfs))
                {
                    int c = readClient(lobby[i], buffer);
                    /* client disconnected */
                    if (c == 0)
                    {
                        closesocket(lobby[i]);
                    }
                    else
                    {
                        char *command = strtok(buffer, " ");
                        if (strcmp(command, "MSG") == 0)
                        {
                            char *msg = strtok(NULL, "");
                            sendMessageToLobby(lobby, actualLobby, msg);
                        }
                        else if (strcmp(command, "LOGIN") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            char *password = strtok(NULL, "");
                            if (username == NULL || password == NULL)
                            {
                                char msg[] =
                                        "Error: No username or password "
                                        "provided. Use: MSG <username> "
                                        "<password>\n";
                                writeClient(lobby[i], msg);
                                continue;
                            }

                            login(clients, &actualClient, connectedClients,
                                  &actualConnected, activeGameSessions,
                                  &numActiveGames, gameSessions, &numGames,
                                  lobby, &actualLobby, i, username, password);
                        }
                        else if (strcmp(command, "SIGNUP") == 0)
                        {
                            char *username = strtok(NULL, " ");
                            char *password = strtok(NULL, "");
                            if (username == NULL || password == NULL)
                            {
                                char msg[] =
                                        "Error: No username or password "
                                        "provided. Use: MSG <username> "
                                        "<password>\n";
                                writeClient(lobby[i], msg);
                                continue;
                            }

                            signUp(clients, &actualClient, connectedClients,
                                   &actualConnected, lobby, &actualLobby, i,
                                   username, password);
                        }
                        else if (strcmp(command, "HELP") == 0)
                        {
                            sendHelp(lobby[i], 0);
                        }
                        else
                        {
                            char msg[] =
                                    "Error: Unknown command. Use HELP to list "
                                    "every known commands\n";
                            writeClient(lobby[i], msg);
                        }
                    }
                    break;
                }
            }
        }
    }

    clearClients(connectedClients, actualConnected);
    endConnection(sock);
}
