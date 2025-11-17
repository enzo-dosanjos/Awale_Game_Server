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
         /* clear input buffer */
         char buffer[BUF_SIZE];
         read(STDIN_FILENO, buffer, BUF_SIZE);

         /* stop process when type on keyboard */
         break;
      }
      else if (FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = {0};
         socklen_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
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

                processClientCommand(buffer, client, clients, &actualClient,
                                     connectedClients, &actualConnected,
                                     gameSessions, &numGames,
                                     activeGameSessions, &numActiveGames);
                break;
            }
         }

         for (i = 0; i < actualLobby; i++)
         {
            if (FD_ISSET(lobby[i], &rdfs))
            {
               processLobbyCommand(buffer, i, clients, &actualClient,
                                    connectedClients, &actualConnected,
                                    lobby, &actualLobby,
                                    gameSessions, &numGames,
                                    activeGameSessions, &numActiveGames);
               break;
            }
         }
      }
   }

   freeServerData(connectedClients, actualConnected, lobby, actualLobby, activeGameSessions, numActiveGames);
   endConnection(sock);
}
