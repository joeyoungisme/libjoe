#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include "shm.h"


const struct option opt_ls[] = {
    {"file2key",    required_argument,  NULL, 0},
    {NULL, 0, NULL, 0}
};

int shm_get_key(char *path)
{
    if (!path) { return -1; }

    if (access(path, F_OK)) {
        printf("file not existed.\n");
        return -1;
    }

    key_t key = ftok(path, 1);
    if (key == -1) {
        printf("%s() ftok() : %s\n", __func__, strerror(errno));
        return -1;
    }

    printf("%s : %X\n", path, key);

    /*
    int id = shmget(key, size??, flag);
    if (shm->id < 0) {
        printf("%s() shmget() : %s\n", __func__, strerror(errno));
        printf("%s() key (%X) size(%d) flag (%X)\n", __func__, shm->key, shm->mem_size, flag);
        return -4;
    }
    */
    return 0;
}

char usage[] = 
"usage : ./user [operation] <args>\n\n"
"[ operation ] :\n"
" --file2key    : give file show key.\n"
"";

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("%s", usage);
        exit(EXIT_FAILURE);
    }

    int opt = 0;
    int opt_idx = 0;
    while((opt = getopt_long(argc, argv, "", opt_ls, &opt_idx)) != -1)
    {
        if (opt != 0) {
            printf("%s", usage);
            exit(EXIT_FAILURE);
        }

        if (!strncmp(opt_ls[opt_idx].name, "file2key", 8)) {

            shm_get_key(optarg);

        }
    }

    exit(EXIT_SUCCESS);
}
