#ifndef AWALE_GAME_SERVER_COMMANDS_H
#define AWALE_GAME_SERVER_COMMANDS_H

#include "../game/gameLogic.h"
#include "../game/ihm.h"
#include "serverUtils.h"

int challenge(Client *clients, Client *challenger, int actual, char username[]);

int acceptChallenge(Client *clients, Client *client, int actual, char challenger[], GameSession *gameSession);

int declineChallenge(Client *clients, Client *client, int actual, char challenger[]);

int move(Client *client, GameSession *gameSessions, int actualGame, int house);

int suggestEndgame(Client *client, GameSession *gameSessions, int actualGame);

int acceptEndgame(Client *client, GameSession *gameSessions, int actualGame);

void handleEndgame(GameSession *gameSession);

void listClients(Client *clients, int actual, Client requester);

void listGames(GameSession *gameSessions, int actualGame, Client requester);

void seePendingReq(Client *client);

void seeSentReq(Client *client);

void clearPendingReq(Client *client);

void clearSentReq(Client *client);

int removeSentReq(Client *clients, Client *client, int actual, char username[]);

void updateBio(Client *client, char bio[]);

int showBio(Client *clients, int actual, Client *requester, char username[]);

int addFriend(Client *client, char username[]);

void setPrivacy(Client *client, int privacy);

#endif //AWALE_GAME_SERVER_COMMANDS_H