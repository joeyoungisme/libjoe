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
#include "timer.h"

int main(int argc, char *argv[])
{
    int role = 0;
    char key[64] = {0};

    int opt = 0;
    while((opt = getopt(argc, argv, "sckd:")) != -1)
    {
        switch(opt)
        {
            case 's':
                break;
            case 'c':
                break;
            case 'k':
                break;
            case 'd':
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    Timer timer = {0};
    tm_set_ms(&timer, 5000);

    char tm_str[32] = {0};
    tm_to_str(&timer, tm_str, sizeof(tm_str));
    PRINT_DEBUG("tm_set_ms : %s\n", tm_str);

    Timer sys_time = {0};
    tm_from_sys(&sys_time);
    tm_to_str(&sys_time, tm_str, sizeof(tm_str));
    PRINT_DEBUG("sys time : %s\n", tm_str);

    timespec_get(&sys_time, TIME_UTC);
    tm_to_str(&sys_time, tm_str, sizeof(tm_str));
    PRINT_DEBUG("sys time : %s\n", tm_str);

    tzset();
    int offset = (int)(-timezone / 60 + (daylight ? 60:0));
    PRINT_DEBUG("offset : %d\n", offset);

    Timer show = {0};
    tm_set_ms(&show, 5000);

    while(1)
    {
        if (tm_is_timeout(timer)) {
            PRINT_DEBUG(" !! timer is timeout !!\n", __func__);
            tm_set_ms(&timer, 5000);

            Timer test = {0};
            clock_gettime(CLOCK_MONOTONIC, &test);
            PRINT_DEBUG("tv_sec : %llu , tv_nsec : %llu\n", (unsigned long long)test.tv_sec, (unsigned long long)test.tv_nsec);

        }

        PRINT_DEBUG("timer stopwatch : %d ms \n", tm_stopwatch(timer));
        PRINT_DEBUG("timer remaining : %d ms \n", tm_remaining(timer));

        sleep(2);
    }

    exit(EXIT_SUCCESS);
}
