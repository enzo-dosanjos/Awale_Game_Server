#ifndef AWALE_GAME_SERVER_GAMESERVER_H
#define AWALE_GAME_SERVER_GAMESERVER_H


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

#include "../constants.h"
#include "commands.h"
#include "serverUtils.h"

#include <errno.h>


void initServer(void);
void endServer(void);
void appServer(void);


#endif //AWALE_GAME_SERVER_GAMESERVER_H