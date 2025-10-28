#include "gameLogic.h"
#include "gameServer.h"

int main(int argc, char **argv)
{
   initServer();

   appServer();

   endServer();

   return EXIT_SUCCESS;
}