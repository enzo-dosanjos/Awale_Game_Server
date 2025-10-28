#ifndef AWALE_GAME_SERVER_CLIENT_H
#define AWALE_GAME_SERVER_CLIENT_H

#ifdef WIN32

#include <winsock2.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif

#define CRLF     "\r\n"
#define PORT     1977

#define BUF_SIZE 1024

void initClient(void);
void endClient(void);
void appClient(const char *address, const char *name);
int init_connectionClient(const char *address);
void end_connection(int sock);
int read_server(SOCKET sock, char *buffer);
void write_server(SOCKET sock, const char *buffer);

#endif //AWALE_GAME_SERVER_CLIENT_H
