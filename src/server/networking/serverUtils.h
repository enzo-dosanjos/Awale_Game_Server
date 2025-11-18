#ifndef AWALE_SERVER_UTILS_GAMESERVER_H
#define AWALE_SERVER_UTILS_GAMESERVER_H

#ifdef WIN32

#include <winsock2.h>

#else

#include <arpa/inet.h>
#include <netdb.h> /* gethostbyname */
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> /* close */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

#endif

#include <errno.h>

#include "../data/data.h"


// Server networking primitives
int initConnectionServer(void);
void endConnection(int sock);

// Client I/O
int readClient(SOCKET sock, char *buffer);

void writeClient(SOCKET sock, const char *buffer);

void askClientInput(SOCKET sock, char *command, char* defaultValue,
                    char *parametersList, char *numParams, char *message);

// Messaging
void sendMessageToAllClients(Client **connectedClients, int actualConnected,
                             char sender[], const char *buffer,
                             char from_server);

void sendMessageToLobby(SOCKET *lobby, int actualLobby,
                        const char *message);

// Client / lobby management
void removeClient(Client **connectedClients, int *actualConnected,
                  int to_remove);

void removeFromLobby(SOCKET *lobby, int *actualLobby, int to_remove);

void clearClients(Client **connectedClients, int actualConnected);

// Time formatting
void formatTime(time_t t, char *out, size_t outSize);

// Cleanup
void freeServerData(Client **connectedClients, int actualConnected, SOCKET *lobby, int actualLobby, GameSession **gameSessions, int numGames);


#endif // AWALE_SERVER_UTILS_GAMESERVER_H