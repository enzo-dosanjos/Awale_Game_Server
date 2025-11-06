#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gameServer.h"


void initServer(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
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
   int actual = 0;
   int max = sock;
   int actualGame = 0;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];
   GameSession gameSessions[MAX_CLIENTS / 2];

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         socklen_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if(readClient(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c;
         c.sock = csock;
         strncpy(c.username, buffer, BUF_SIZE - 1);
         c.gameId = NULL;
         c.numFriends = 0;
         c.numPendingChallengesTo = 0;
         c.numPendingChallengesFrom = 0;
         c.bio[0] = '\0';
         c.private = 0;
         clients[actual] = c;
         actual++;
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client *client = &clients[i];
               int c = readClient(clients[i].sock, buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i].sock);
                  removeClient(clients, i, &actual);
                  strncpy(buffer, client->username, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  sendMessageToAllClients(clients, *client, actual, buffer, 1);
               }
               else
               {
                  char *command = strtok(buffer, " ");
                  if (strcmp(command, "CHALLENGE") == 0)
                  {
                     char *username = strtok(NULL, " ");
                     challenge(clients, client, actual, username);
                  }
                  else if (strcmp(command, "ACCEPT") == 0)
                  {
                     char *username = strtok(NULL, " ");
                     GameSession newGameSession;
                     if (acceptChallenge(clients, client, actual, username, &newGameSession)) {
                        gameSessions[actualGame] = newGameSession;
                        actualGame++;
                     }
                  }
                  else if (strcmp(command, "DECLINE") == 0)
                  {
                     char *username = strtok(NULL, " ");
                     declineChallenge(clients, client, actual, username);
                  }
                  else if (strcmp(command, "LIST") == 0)
                  {
                     listClients(clients, actual, *client);
                  }
                  else if (strcmp(command, "LISTGAMES") == 0)
                  {
                     listGames(gameSessions, actualGame, *client);
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
                     removeSentReq(clients, client, actual, username);
                  }
                  else if (strcmp(command, "MOVE") == 0)
                  {
                     char *houseChar = strtok(NULL, " ");
                     int house = atoi(houseChar);
                     move(client, gameSessions, actualGame, house);
                  }
                  else if (strcmp(command, "ENDGAME") == 0)
                  {
                     suggestEndgame(client, gameSessions, actualGame);
                  }
                  else if (strcmp(command, "ACCEPTEND") == 0)
                  {
                     acceptEndgame(client, gameSessions, actualGame);
                  }
                  else if (strcmp(command, "MSG") == 0)
                  {
                     // check if it's a private message by checking the number of tokens
                     char *msgOrUsername = strtok(NULL, " ");
                     char *msgOrNull = strtok(NULL, " ");
                     if (msgOrNull != NULL) {
                        // It's a private message
                        char *username = msgOrUsername;
                        char *message = msgOrNull;
                        sendMessageToClient(clients, client, actual, username, message);
                     } else {
                        // It's a public message
                        char *message = msgOrUsername;
                        sendMessageToAllClients(clients, *client, actual, message, 0);
                     }
                  }
                  else if (strcmp(command, "ADDFRIEND") == 0)
                  {
                     char *username = strtok(NULL, " ");
                     addFriend(client, username);
                  }
                  else if (strcmp(command, "BIO") == 0)
                  {
                     char *bio = strtok(NULL, " ");
                     updateBio(client, bio);
                  }
                  else if (strcmp(command, "SHOWBIO") == 0)
                  {
                     char *username = strtok(NULL, " ");
                     showBio(clients, actual, client, username);
                  }
                  else if (strcmp(command, "SETPRIVACY") == 0)
                  {
                     char *privacyStr = strtok(NULL, " ");
                     if (strcmp(privacyStr, "true") == 0) {
                        setPrivacy(client, 1);
                     } else {
                        setPrivacy(client, 0);
                     }
                  }
               }
               break;
            }
         }
      }
   }

   clearClients(clients, actual);
   endConnection(sock);
}
