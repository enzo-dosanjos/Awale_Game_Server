#ifndef AWALE_GAME_SERVER_COMMANDS_H
#define AWALE_GAME_SERVER_COMMANDS_H

#include "../game/gameLogic.h"
#include "../game/ihm.h"
#include "serverUtils.h"

int signUp(Client *clients, int *actualClient, Client **connectedClients,
           int *actualConnected, SOCKET *lobby, int *actualLobby, int index,
           char *username, char *password);

int login(Client *clients, int *actualClient, Client **connectedClients,
          int *actualConnected, GameSession **activeGameSessions,
          int *numActiveGames, GameSession *gameSessions, int *numGames,
          SOCKET *lobby, int *actualLobby, int index, char *username,
          char *password);

int challenge(Client **connectedClients, Client *challenger,
              int actualConnected, char username[]);

int acceptChallenge(Client **connectedClients, Client *client,
                    int actualConnected, char challenger[],
                    GameSession *gameSessions, int *numGames,
                    GameSession **activeGameSessions, int *numActiveGames);

int declineChallenge(Client **connectedClients, Client *client,
                     int actualConnected, char challenger[]);

int move(Client *client, GameSession **activeGameSessions, int *numActiveGames,
         GameSession *gameSessions, int *numGames, int house);

int suggestEndgame(Client *client, GameSession **activeGameSessions,
                   int *numActiveGames, GameSession *gameSessions,
                   int *numGames);

int acceptEndgame(Client *client, GameSession **activeGameSessions,
                  int *numActiveGames, GameSession *gameSessions,
                  int *numGames);

void handleEndgame(GameSession *gameSession, GameSession **activeGameSessions,
                   int *numActiveGames, GameSession *gameSessions,
                   int *numGames);

void listClients(Client **connectedClients, int actualConnected,
                 Client requester);

void listGames(GameSession **gameSessions, int actualGame, Client requester);

void seePendingReq(Client *client);

void seeSentReq(Client *client);

void clearPendingReq(Client *client);

void clearSentReq(Client *client);

int removeSentReq(Client **connectedClients, Client *client,
                  int actualConnected, char username[]);

void sendMP(Client **connectedClients, Client *sender, int actualConnected,
            char *username, char *message);

void updateBio(Client *client, char bio[]);

int showBio(Client **connectedClients, int actualConnected, Client *requester,
            char username[]);

int showStats(Client **connectedClients, int actualConnected, Client *requester,
              char username[]);

int addFriend(Client *client, char username[]);

void setPrivacy(Client *client, int privacy);

int watchGame(Client *client, GameSession **gameSessions, int actualGame,
              int gameId);

int SendMsgGame(GameSession *gameSession, Client *sender, char *message);

int quit(Client **connectedClients, int *actualConnected, Client *client,
         GameSession **activeGameSessions, int *numActiveGames,
         GameSession *gameSessions, int *numSavedGames);

int loadGame(Client **connectedClients, int actualConnected, Client *client,
             GameSession **activeGameSessions, int *numActiveGames,
             GameSession *gameSessions, int *numGames);

void sendHelp(SOCKET sock, int loggedIn);

int saveGameAndSend(Client *client, GameSession **activeGameSessions,
                    int numActiveGames);

#endif // AWALE_GAME_SERVER_COMMANDS_H