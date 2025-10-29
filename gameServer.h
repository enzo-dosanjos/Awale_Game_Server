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

#include "constants.h"
#include "gameLogic.h"

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100

#define BUF_SIZE    1024


typedef struct
{
    SOCKET sock;
    char username[BUF_SIZE];
    char bio[BUF_SIZE];
    int private;
    int *gameId;
} Client;

typedef struct
{
    int id;
    Client players[NUM_PLAYERS];
    Game game;
    int currentPlayer;
} GameSession;

void initServer(void);
void endServer(void);
void appServer(void);
int init_connectionServer(void);
void end_connection(int sock);
int read_client(SOCKET sock, char *buffer);
void write_client(SOCKET sock, const char *buffer);
void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
Client *findClientByUsername(Client *clients, int actual, char username[]);
void sendMessageToClient(Client *clients, int actual, char username[], const char *buffer);
void remove_client(Client *clients, int to_remove, int *actual);
void clear_clients(Client *clients, int actual);


#endif //AWALE_GAME_SERVER_GAMESERVER_H