#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "serverUtils.h"

int initConnectionServer(void)
// Initializes the server socket, binds it to the specified port, and starts listening for incoming connections.
// Errors : socket creation failure, bind failure, listen failure
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = {0};

   if (sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

void endConnection(int sock)
// Closes the server socket.
// Errors : none
{
   closesocket(sock);
}

void clearClients(Client **connectedClients, int actualConnected)
// Closes the sockets of all connected clients.
// Errors : none
{
   int i = 0;
   for (i = 0; i < actualConnected; i++)
   {
      closesocket(connectedClients[i]->sock);
   }
}

void removeClient(Client **connectedClients, int *actualConnected,
                  int to_remove)
// Removes a client from the connected clients array.
// Errors : none
{
   /* we remove the client in the array */
   memmove(connectedClients + to_remove, connectedClients + to_remove + 1, (*actualConnected - to_remove - 1) * sizeof(Client *));
   /* number client - 1 */
   (*actualConnected)--;
}

void removeFromLobby(SOCKET *lobby, int *actualLobby, int to_remove)
// Removes a socket from the lobby array.
// Errors : none
{
   memmove(lobby + to_remove, lobby + to_remove + 1, (*lobby - to_remove - 1) * sizeof(SOCKET));
   (*actualLobby)--;
}

void sendMessageToAllClients(Client **connectedClients, int actualConnected,
                             char sender[], const char *buffer,
                             char from_server)
// Sends a message to all connected clients, optionally including the sender's name.
// Errors : none
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;

   if (from_server == 0)
   {
      strncpy(message, sender, BUF_SIZE - 1);
      strncat(message, " : ", sizeof message - strlen(message) - 1);
   }
   strncat(message, buffer, sizeof message - strlen(message) - 1);

   for (i = 0; i < actualConnected; i++)
   {
      if (connectedClients[i] != NULL)
      {
         writeClient(connectedClients[i]->sock, message);
      }
   }
}

void sendMessageToLobby(SOCKET *lobby, int actualLobby, const char *message)
// Sends a message to all sockets in the lobby.
// Errors : none
{
   for (int i = 0; i < actualLobby; i++)
   {
      writeClient(lobby[i], message);
   }
}

void formatTime(time_t t, char *out, size_t outSize)
// Formats a time_t value into a human-readable string.
// Errors : none
{
    struct tm *lt = localtime(&t);
    if (lt)
    {
        strftime(out, outSize, "%Y-%m-%d %H:%M:%S", lt);
    }
    else
    {
        strncpy(out, "unknown", outSize - 1);
        out[outSize - 1] = '\0';
    }
}

int readClient(SOCKET sock, char *buffer)
// Reads a message from the specified client socket into the buffer.
// Returns the number of bytes read.
// Errors : recv failure
{
   int n = 0;

   if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

void writeClient(SOCKET sock, const char *buffer)
// Sends a message to the specified client socket.
// Errors : send failure
{
   if (send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

void freeServerData(Client **connectedClients, int actualConnected,
                    SOCKET *lobby, int actualLobby, GameSession **gameSessions,
                    int numGames)
// Frees all dynamically allocated server data, including connected clients, lobby sockets, and game sessions.
// Errors : none
{
    int i;
    // Close all client sockets
    clearClients(connectedClients, actualConnected);

    for (i = 0; i < actualLobby; i++)
    {
        closesocket(lobby[i]);
    }

    // Free all dynamically allocated game data
    for (i = 0; i < numGames; i++)
    {
        freeGame(&gameSessions[i]->game);
    }
}

