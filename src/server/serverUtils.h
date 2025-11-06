#ifndef AWALE_SERVER_UTILS_GAMESERVER_H
#define AWALE_SERVER_UTILS_GAMESERVER_H


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

#include "../game/gameUtils.h"
#include "../constants.h"

#include <errno.h>


typedef struct
{
    SOCKET sock;
    char username[BUF_SIZE];
    char bio[BUF_SIZE];
    int private;
    int *gameId;
    // friends list
    int numFriends;
    char friends[MAX_FRIENDS][BUF_SIZE];
    // sent challenges to
    int numPendingChallengesTo;
    char pendingChallengesTo[MAX_PENDING_CHALLENGES][BUF_SIZE];
    // received challenges from
    int numPendingChallengesFrom;
    char pendingChallengesFrom[MAX_PENDING_CHALLENGES][BUF_SIZE];
} Client;

typedef struct
{
    int id;
    Client *players[NUM_PLAYERS];
    Game game;
    int currentPlayer;
    int endGameSuggested;
} GameSession;

void initServer(void);
void endServer(void);
void appServer(void);
int initConnectionServer(void);
void endConnection(int sock);
int readClient(SOCKET sock, char *buffer);
void writeClient(SOCKET sock, const char *buffer);
void sendMessageToAllClients(Client *clients, Client client, int actual, const char *buffer, char from_server);
Client *findClientByUsername(Client *clients, int actual, char username[]);
GameSession *findGameSessionByClient(Client *client, GameSession *gameSessions, int actualGame);
void sendMessageToClient(Client *clients, Client *sender, int actual, char username[], const char *buffer);
void removeClient(Client *clients, int to_remove, int *actual);
void clearClients(Client *clients, int actual);
int addChallenge(Client *challenger, Client *challenged);
int removeChallenge(Client *client, Client *challenged);
void clearSentChallenge(Client *client);
void clearReceivedChallenge(Client *client);


#endif //AWALE_SERVER_UTILS_GAMESERVER_H