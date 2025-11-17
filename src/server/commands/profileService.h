#ifndef AWALE_GAME_SERVER_USERSERVICE_H
#define AWALE_GAME_SERVER_USERSERVICE_H


#include "../networking/serverUtils.h"
#include "../dataManagers/clientManager.h"
#include "../dataManagers/gameSessionManager.h"


void profile_updateBio(Client *client, char bio[]);

int profile_showBio(Client *clients, int actualClient,
                    Client *requester, char username[]);

int profile_showStats(Client *clients, int actualClient,
                      Client *requester, char username[]);

int profile_addFriend(Client *clients, int actualClient,
                      Client *client, char username[]);

int profile_removeFriend(Client *client, char username[]);

int profile_showFriends(Client *clients, int actualClient,
                        Client *requester, char username[]);

void profile_setPrivacy(Client *client, int privacy);

#endif //AWALE_GAME_SERVER_USERSERVICE_H
