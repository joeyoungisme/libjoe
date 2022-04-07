#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <sys/sem.h>

#include "mem.h"
#include "misc.h"
#include "sema.h"
#include "timer.h"

int main(int argc, char *argv[])
{
    int role = 0;
    int delay = 0;
    char key[64] = {0};

    int opt = 0;
    while((opt = getopt(argc, argv, "c:s:k:")) != -1)
    {
        switch(opt)
        {
            case 's':
                role = 1;
                delay = atoi(optarg);
                break;
            case 'c':
                role = 0;
                delay = atoi(optarg);
                break;
            case 'k':
                snprintf(key, 64, "%s", optarg);
                break;
            default:
                printf("usage : %s [-c|-s] [-k /tmp/test.key]\nc : client\ns : server\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!strlen(key)) {
        printf("usage : %s [-c|-s] [-k /tmp/test.key]\nc : client\ns : server\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int init = SEMA_INIT_EXCEP_LOCK;
    if (role) init = SEMA_INIT_FORCE_UNLOCK;
    sema_class *sema = sema_new();
    if (!sema) {
        PRINT_ERR("%s() sema new failed\n", __func__);
        exit(EXIT_FAILURE);
    } else if (sema->init(sema, key, init)) {
        PRINT_ERR("%s() sema init failed\n", __func__);
        exit(EXIT_FAILURE);
    }

    int count = 0;
    while(1)
    {
        sema->lock(sema);

        printf("%d : poll %d times , lock %d sec\n", getpid(), count++, delay);

        struct timespec timer = {0};
        tm_set_ms(&timer, delay * 1000);

        while(!tm_is_timeout(timer)) usleep(1000);

        sema->unlock(sema);
    }

    exit(EXIT_SUCCESS);
}
