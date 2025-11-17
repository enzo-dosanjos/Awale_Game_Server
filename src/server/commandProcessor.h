#ifndef AWALE_GAME_SERVER_COMMANDPROCESSOR_H
#define AWALE_GAME_SERVER_COMMANDPROCESSOR_H


#include "networking/serverUtils.h"
#include "commands/chatService.h"
#include "commands/gameService.h"
#include "commands/challengeService.h"
#include "commands/generalService.h"
#include "commands/accountService.h"
#include "commands/profileService.h"


// Process a command from a logged-in client.
void processClientCommand(char *buffer,
                          Client *client,
                          Client *clients, const int *actualClient,
                          Client **connectedClients, int *actualConnected,
                          GameSession *gameSessions, int *numGames,
                          GameSession **activeGameSessions,
                          int *numActiveGames);

// Process a command from a lobby socket (not logged in yet).
void processLobbyCommand(char *buffer,
                         int lobbyIndex,
                         Client *clients, int *actualClient,
                         Client **connectedClients, int *actualConnected,
                         SOCKET *lobby, int *actualLobby,
                         GameSession *gameSessions, int *numGames,
                         GameSession **activeGameSessions,
                         int *numActiveGames);


#endif //AWALE_GAME_SERVER_COMMANDPROCESSOR_H
