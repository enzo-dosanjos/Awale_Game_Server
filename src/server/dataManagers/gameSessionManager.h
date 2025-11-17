#ifndef AWALE_GAME_SERVER_GAMESESSIONSMANAGER_H
#define AWALE_GAME_SERVER_GAMESESSIONSMANAGER_H

#include "../data/data.h"


GameSession *initGameSession(GameSession *gameSessions, int *numGameSessions,
                             Game *game, int firstPlayer, Client *player1,
                             Client *player2);

// gameSessions management
int removeGameSession(GameSession *gameSessions, int *numGames, int gameId);

int removeActiveGameSession(GameSession **activeGameSessions, int *numGames,
                            int gameId);
// Recording
void recordMove(GameSession *gameSession, const Move *move, const char *grid);

void recordChat(GameSession *gameSession, const char *sender, const char *text);

// Lookups
GameSession *findGameSessionByClient(Client *client,
                                     GameSession **gameSessions,
                                     int actualGame);
GameSession *findGameSessionByViewer(GameSession **gameSessions,
                                     int actualGame, Client *viewer);

#endif //AWALE_GAME_SERVER_GAMESESSIONSMANAGER_H
