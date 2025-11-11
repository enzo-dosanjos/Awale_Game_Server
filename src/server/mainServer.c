#include "gameServer.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    initServer();

    appServer();

    endServer();

    return EXIT_SUCCESS;
}
