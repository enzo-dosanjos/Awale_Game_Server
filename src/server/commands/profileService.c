#include <stdio.h>
#include <string.h>

#include "profileService.h"


void profile_updateBio(Client *client, char bio[])
// Updates the bio of the specified client.
// Errors : none
{
    strncpy(client->bio, bio, BUF_SIZE - 1);
    char msg[] = "Bio updated successfully.\n";
    writeClient(client->sock, msg);
}

int profile_showBio(Client *clients, int actualClient,
                    Client *requester, char username[])
// Displays the bio of the specified user or the requester if no username is provided.
// Errors : user not found, private bio access denied
{
    Client *client;
    if (username == NULL || strlen(username) == 0)
    {
        client = requester;
    }
    else
    {
        client = findClientByUsername(clients, actualClient, username);
        if (client == NULL)
        {
            char msg[] = "Error: User not found.\n";
            writeClient(requester->sock, msg);
            return 0;
        }

        if (client->private)
        {
            // check if requester is a friend
            int found = 0;
            for (int i = 0; i < client->numFriends; i++)
            {
                if (strcmp(client->friends[i], requester->username) == 0)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                char msg[] = "Error: This user's bio is private.\n";
                writeClient(requester->sock, msg);
                return 0;
            }
        }
    }

    char message[3 * BUF_SIZE];
    snprintf(message, 3 * BUF_SIZE, "Bio of %s:\n%s", client->username, client->bio);
    writeClient(requester->sock, message);
    return 1;
}

int profile_showStats(Client *clients, int actualClient,
                      Client *requester, char username[])
// Displays the statistics of the specified user or the requester if no username is provided.
// Errors : user not found, private stats access denied
{
    Client *client;
    if (username == NULL || strlen(username) == 0)
    {
        client = requester;
    }
    else
    {
        client = findClientByUsername(clients, actualClient, username);
        if (client == NULL)
        {
            char msg[] = "Error: User not found.\n";
            writeClient(requester->sock, msg);
            return 0;
        }

        if (client->private)
        {
            // check if requester is a friend
            int found = 0;
            for (int i = 0; i < client->numFriends; i++)
            {
                if (strcmp(client->friends[i], requester->username) == 0)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                char msg[] = "Error: This user's bio is private.\n";
                writeClient(requester->sock, msg);
                return 0;
            }
        }
    }

    char message[3 * BUF_SIZE];
    snprintf(message, 3 * BUF_SIZE, "Stats of %s:\n\tGames played: %d\n\tGames won: %d\n\tGames lost: %d\n\tGames drawn: %d\n\tAverage number of moves to win: %.0f\n\tTotal number of seeds collected: %d\n", client->username, client->stats.gamesPlayed, client->stats.gamesWon, client->stats.gamesLost, client->stats.gamesDrawn, client->stats.averageMovesToWin, client->stats.totalSeedsCollected);
    writeClient(requester->sock, message);
    return 1;
}

int profile_addFriend(Client *clients, int actualClient,
                      Client *client, char username[])
// Adds a friend to the client's friend list.
// Errors : adding self, user not found, already friends, friend list full
{
    if (strcmp(client->username, username) == 0)
    {
        char msg[] = "Error: You cannot add yourself as a friend.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    Client *friend = findClientByUsername(clients, actualClient, username);
    if (friend == NULL)
    {
        char msg[] = "Error: User not found.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    // check if already friends
    for (int i = 0; i < client->numFriends; i++)
    {
        if (strcmp(client->friends[i], username) == 0)
        {
            char msg[] = "Error: This user is already your friend.\n";
            writeClient(client->sock, msg);
            return 0;
        }
    }

    // check if friend list is full
    if (client->numFriends >= MAX_FRIENDS)
    {
        char msg[] = "Error: Friend list is full.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    strncpy(client->friends[client->numFriends], username, BUF_SIZE - 1);
    client->numFriends++;

    return 1;
}

int profile_removeFriend(Client *client, char username[])
// Removes a friend from the client's friend list.
// Errors : user not in friend list
{
    int found = 0;
    for (int i = 0; i < client->numFriends; i++)
    {
        if (strcmp(client->friends[i], username) == 0)
        {
            found = 1;
            // shift friends down
            for (int j = i; j < client->numFriends - 1; j++)
            {
                strcpy(client->friends[j], client->friends[j + 1]);
            }
            client->numFriends--;
            break;
        }
    }

    if (!found)
    {
        char msg[] = "Error: This user is not your friend.\n";
        writeClient(client->sock, msg);
        return 0;
    }

    return 1;
}

int profile_showFriends(Client *clients, int actualClient,
                        Client *requester, char username[])
// Displays the friend list of the specified user or the requester if no username is provided.
// Errors : user not found, private friends list access denied
{
    Client *client;
    if (username == NULL || strlen(username) == 0)
    {
        client = requester;
    }
    else
    {
        client = findClientByUsername(clients, actualClient, username);
        if (client == NULL)
        {
            char msg[] = "Error: User not found.\n";
            writeClient(requester->sock, msg);
            return 0;
        }

        if (client->private)
        {
            // check if requester is a friend
            int found = 0;
            for (int i = 0; i < client->numFriends; i++)
            {
                if (strcmp(client->friends[i], requester->username) == 0)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                char msg[] = "Error: This user's friends list is private.\n";
                writeClient(requester->sock, msg);
                return 0;
            }
        }
    }

    char message[2*BUF_SIZE];

    sprintf(message, "Friends of %s:\n", client->username);
    if (client->numFriends == 0)
    {
        strcat(message, "No friends added yet.\n");
    }
    else
    {
        for (int i = 0; i < client->numFriends; i++)
        {
            strcat(message, client->friends[i]);
            strcat(message, "\n");
        }
    }

    writeClient(requester->sock, message);
    return 1;
}

void profile_setPrivacy(Client *client, int privacy)
// Sets the privacy setting of the specified client.
// Errors : none
{
    client->private = privacy;
    char msg[] = "Privacy setting updated.\n";
    writeClient(client->sock, msg);
}
