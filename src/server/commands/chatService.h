#ifndef AWALE_GAME_SERVER_CHATSERVICE_H
#define AWALE_GAME_SERVER_CHATSERVICE_H


#include "../../constants.h"
#include "../dataManagers/clientManager.h"
#include "../dataManagers/gameSessionManager.h"
#include "../networking/serverUtils.h"


// Lobby & global chat
void chat_broadcastToAll(Client **connectedClients, int actualConnected,
                         Client *sender, const char *message);

// Lobby chat (not logged in)
void chat_broadcastToLobby(SOCKET *lobby, int actualLobby, const char *message);

// Private message between connected users
void chat_sendPrivate(Client **connectedClients, int actualConnected,
                      Client *sender, char *targetUsername,
                      const char *message);

// In-game chat (players + viewers)
int chat_sendGameMessage(GameSession *gameSession, Client *sender,
                         const char *message);


#endif //AWALE_GAME_SERVER_CHATSERVICE_H
