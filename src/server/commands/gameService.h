#ifndef AWALE_GAME_SERVER_GAMESERVICE_H
#define AWALE_GAME_SERVER_GAMESERVICE_H

#include "../networking/serverUtils.h"
#include "../dataManagers/clientManager.h"
#include "../dataManagers/gameSessionManager.h"
#include "../../game/gameLogic.h"
#include "../../game/ihm.h"


int game_start(Client *client, Client **connectedClients,
               int actualConnected, char challenger[],
               GameSession *gameSessions, int *numGames,
               GameSession **activeGameSessions, int *numActiveGames,
               int rotation);

// In-game actions
int game_move(Client *client,
              GameSession **activeGameSessions, int *numActiveGames,
              int house);

int game_suggestEnd(Client *client,
                    GameSession **activeGameSessions, int *numActiveGames);

int game_acceptEnd(Client *client,
                   GameSession **activeGameSessions, int *numActiveGames);

void game_handleEndForPlayer(Client *client,
                             Client **connectedClients, int actualConnected,
                             GameSession **activeGameSessions,
                             int *numActiveGames,
                             GameSession *gameSessions, int *numGames,
                             int saveFlag);

// Load / save games
int game_loadLast(Client **connectedClients, int actualConnected,
                  Client *client,
                  GameSession **activeGameSessions, int *numActiveGames,
                  GameSession *gameSessions, int *numGames);

int game_saveAndSend(Client *client, GameSession **activeGameSessions,
                     int numActiveGames);

// Spectating
int game_watch(Client *client, GameSession **gameSessions, int actualGame,
                    int gameId);

#endif //AWALE_GAME_SERVER_GAMESERVICE_H
