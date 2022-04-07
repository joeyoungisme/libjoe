#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include <sys/timeb.h>

#include "misc.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PRINT_DEBUG("%s invalid args.\n", argv[0]);
        PRINT_DEBUG("%s [execute command]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char cmd[1024] = {0};
    int cnt = 0;
    for (int idx = 1; idx < argc; ++idx) {
        cnt += snprintf(cmd + cnt, 1024 - cnt, "%s ", argv[idx]);
    }
    PRINT_DEBUG("exe_cmd : %s\n", cmd);
    int res = exe_cmd(cmd);
    PRINT_DEBUG("result : %d , errno : %d.\n", res, errno);
    exit(EXIT_SUCCESS);
}

