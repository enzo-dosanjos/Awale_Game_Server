#ifndef AWALE_GAME_SERVER_CLIENT_H
#define AWALE_GAME_SERVER_CLIENT_H

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
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif


void initClient(void);
void endClient(void);
void appClient(const char *address);
int initConnectionClient(const char *address);
void endConnection(int sock);
int readServer(SOCKET sock, char *buffer);
void writeServer(SOCKET sock, const char *buffer);
static int endsWithEndMarker(const char *buffer, size_t *posBeforeEnd);
int receiveGameAndSave(SOCKET sock, const char *filename);


#endif // AWALE_GAME_SERVER_CLIENT_H
