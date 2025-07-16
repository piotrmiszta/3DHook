#include "server.h"
#include "worker.h"
#include <stdio.h>
#include <str.h>
#include <unistd.h>

int main(int, char **)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    Server server;
    str_view_t str = STRING_VIEW_CSTR("Hello in 3DHook application!");
    string_view_fprintf(stdout, &str);
    printf("\n");
    s32 err = server_boot(&server);
    if (err)
    {
        printf("Error\n");
        return -1;
    }
    sleep(100);
    worker_close();
    server_close(&server);
    return 0;
}
