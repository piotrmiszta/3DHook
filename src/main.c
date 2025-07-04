#include "server.h"
#include <stdio.h>

int main(int, char **)
{
    Server server;
    int32_t err = server_boot(&server);
    if (err)
    {
        printf("Error\n");
        return -1;
    }
    while (1)
    {
    }
    return 0;
}
