#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

void initClient(void)
{
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err < 0)
    {
        puts("WSAStartup failed !");
        exit(EXIT_FAILURE);
    }
#endif
}

void endClient(void)
{
#ifdef WIN32
    WSACleanup();
#endif
}

void appClient(const char *address)
{
    SOCKET sock = initConnectionClient(address);
    char buffer[BUF_SIZE];

    fd_set rdfs;

    while (1)
    {
        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* add the socket */
        FD_SET(sock, &rdfs);

        if (select(sock + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if (FD_ISSET(STDIN_FILENO, &rdfs))
        {
            fgets(buffer, BUF_SIZE - 1, stdin);
            {
                char *p = NULL;
                p = strstr(buffer, "\n");
                if (p != NULL)
                {
                    *p = 0;
                }
                else
                {
                    /* fclean */
                    buffer[BUF_SIZE - 1] = 0;
                }
            }
            writeServer(sock, buffer);
        }
        else if (FD_ISSET(sock, &rdfs))
        {
            int n = readServer(sock, buffer);

            if (strncmp(buffer, "BEGIN_SAVED_GAME", 16) == 0)
            {
                // read filename
                char filename[BUF_SIZE];
                filename[0] = '\0';
                if (sscanf(buffer, "BEGIN_SAVED_GAME %s", filename) != 1) {
                    printf("Failed to save game.\n");
                    continue;
                }

                // receive game file
                if (receiveGameAndSave(sock, filename)) {
                    printf("Game saved successfully at %s.\n", filename);
                } else {
                    printf("Failed to save game.\n");
                }
                continue;
            }

            /* server down */
            if (n == 0)
            {
                printf("You've been disconnected !\n");
                break;
            }
            puts(buffer);
        }
    }

    endConnection(sock);
}

int initConnectionClient(const char *address)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = {0};
    struct hostent *hostinfo;

    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    hostinfo = gethostbyname(address);
    if (hostinfo == NULL)
    {
        fprintf(stderr, "Unknown host %s.\n", address);
        exit(EXIT_FAILURE);
    }

    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr_list[0];
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        exit(errno);
    }

    return sock;
}

void endConnection(int sock) { closesocket(sock); }

int readServer(SOCKET sock, char *buffer)
{
    int n = 0;

    if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
    {
        perror("recv()");
        exit(errno);
    }

    buffer[n] = 0;

    return n;
}

void writeServer(SOCKET sock, const char *buffer)
{
    if (send(sock, buffer, strlen(buffer), 0) < 0)
    {
        perror("send()");
        exit(errno);
    }
}

static int endsWithEndMarker(const char *buffer, size_t *posBeforeEnd)
{
    // look for "END_SAVED_GAME"
    const char *p = strstr(buffer, "END_SAVED_GAME");

    // end marker not found
    if (!p)
    {
        return 0;
    }

    // calculate position before the end marker
    size_t ind = (size_t)(p - buffer);
    if (posBeforeEnd)
    {
        *posBeforeEnd = ind;
    }

    return 1;
}

int receiveGameAndSave(SOCKET sock, const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Failed to create save file.\n");
        return 0;
    }

    char buf[2*BUF_SIZE];
    int n;
    while ((n = readServer(sock, buf)) > 0)
    {
        size_t beforeEnd = 0;
        if (endsWithEndMarker(buf, &beforeEnd))
        {
            // Write up to the position before the end marker
            if (file && beforeEnd > 0)
            {
                fwrite(buf, 1, beforeEnd, file);
            }
            break;
        }

        // If there's no end marker, write the entire buffer
        if (file && n > 0) {
            fwrite(buf, 1, n, file);
        }
    }

    fclose(file);

    return 1;
}


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage : %s [address]\n", argv[0]);
        return EXIT_FAILURE;
    }

    initClient();

    appClient(argv[1]);

    endClient();

    return EXIT_SUCCESS;
}
