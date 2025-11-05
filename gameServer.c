#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "gameServer.h"
#include "commands.h"

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
   SOCKET sock = init_connectionServer();
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
         if(read_client(csock, buffer) == -1)
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
               int c = read_client(clients[i].sock, buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client->username, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, *client, actual, buffer, 1);
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
                        send_message_to_all_clients(clients, *client, actual, message, 0);
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

   clear_clients(clients, actual);
   end_connection(sock);
}

void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      if(from_server == 0)
      {
         strncpy(message, sender.username, BUF_SIZE - 1);
         strncat(message, " : ", sizeof message - strlen(message) - 1);
      }
      strncat(message, buffer, sizeof message - strlen(message) - 1);
      write_client(clients[i].sock, message);
   }
}

Client *findClientByUsername(Client *clients, int actual, char username[])
{
   for (int i = 0; i < actual; i++)
   {
      if (strcmp(clients[i].username, username) == 0)
      {
         return &clients[i];
      }
   }
   return NULL; // Not found
}

void sendMessageToClient(Client *clients, Client *sender, int actual, char username[], const char *buffer)
{
   char message[BUF_SIZE];
   message[0] = '\0';
   if (sender != NULL) {
      strncpy(message, sender->username, BUF_SIZE - 1);
      strncat(message, " : ", sizeof message - strlen(message) - 1);
   }

   Client *client = findClientByUsername(clients, actual, username);
   if (client != NULL) {
      strncat(message, buffer, sizeof message - strlen(message) - 1);
      write_client(client->sock, message);
   }
}

int init_connectionServer(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

void end_connection(int sock)
{
   closesocket(sock);
}

int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int add_challenge(Client *challenger, Client *challenged) {
   // check if already challenged
   for (int i = 0; i < challenger->numPendingChallengesTo; i++) {
      if (strcmp(challenger->pendingChallengesTo[i], challenged->username) == 0) {
         char msg[] = "Error: You have already challenged this user.";
         write_client(challenger->sock, msg);
         return 0;
      }
   }

   // max pending challenges check
   if (challenger->numPendingChallengesTo >= MAX_PENDING_CHALLENGES) {
      char msg[] = "Error: You have reached the maximum number of pending challenges. Please wait for one to be accepted.";
      write_client(challenger->sock, msg);
      return 0;
   }

   // max pending challenges check for challenged
   if (challenger->numPendingChallengesFrom >= MAX_PENDING_CHALLENGES) {
      char msg[] = "Error: The user you are trying to challenge has reached the maximum number of pending challenges. Please try again later.";
      write_client(challenged->sock, msg);
      return 0;
   }

   strcpy(challenger->pendingChallengesTo[challenger->numPendingChallengesTo], challenged->username);
   challenger->numPendingChallengesTo++;

   strcpy(challenged->pendingChallengesFrom[challenged->numPendingChallengesFrom], challenger->username);
   challenged->numPendingChallengesFrom++;

   return 1;
}

int remove_challenge(Client *client, Client *challenged) {
   int found = 0;
   for (int i = 0; i < client->numPendingChallengesTo; i++) {
      if (strcmp(client->pendingChallengesTo[i], challenged->username) == 0) {
         // Shift remaining challenges down
         for (int j = i; j < client->numPendingChallengesTo - 1; j++) {
            strcpy(client->pendingChallengesTo[j], client->pendingChallengesTo[j + 1]);
         }
         client->numPendingChallengesTo--;
         found = 1;
         break;
      }
   }

   if (!found) {
      char msg[] = "Error: No pending challenge from that user.";
      write_client(client->sock, msg);
      return 0;
   }

   for (int i = 0; i < challenged->numPendingChallengesFrom; i++) {
      if (strcmp(challenged->pendingChallengesFrom[i], client->username) == 0) {
         // Shift remaining challenges down
         for (int j = i; j < challenged->numPendingChallengesFrom - 1; j++) {
            strcpy(challenged->pendingChallengesFrom[j], challenged->pendingChallengesFrom[j + 1]);
         }
         challenged->numPendingChallengesFrom--;
         break;
      }
   }

   return 1;
}

void clear_sent_challenge(Client *client) {
   client->numPendingChallengesTo = 0;
}

void clear_received_challenge(Client *client) {
   client->numPendingChallengesTo = 0;
}
