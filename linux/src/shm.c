#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// for open()
#include <fcntl.h>
#include <sys/stat.h>

// for shmget ... etc.
#include <sys/shm.h>

#include "misc.h"
#include "mem.h"
#include "shm.h"

int shm_destory(shm_class *shm)
{
    if (!shm) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (shmctl(shm->id, IPC_RMID, NULL)) {
        PRINT_ERR("%s() shmctl IPC RMID : %s\n", __func__, strerror(errno));
        return -1;
    }

    ofree(shm);

    return 0;
}
int shm_init(shm_class *shm, char *path, unsigned int size)
{
    if (!shm || !path || !size) { return -1; }

    if (shm->init != shm_init) {
        memset(shm, 0, sizeof(shm_class));
        shm->init = shm_init;
    }

    memset(shm->key_path, 0, SHM_KEY_PATH_LEN);
    snprintf(shm->key_path, SHM_KEY_PATH_LEN, "%s", path);

    shm->mem_size = size;

    int flag = 0777;

    // check specific key file exist
    if (access(shm->key_path, F_OK)) {

        // create new key file
        int fd = open(shm->key_path, (O_CREAT | O_RDWR), 0777);
        if (fd < 0) {
            PRINT_ERR("%s() open() : %s\n", __func__, strerror(errno));
            return -2;
        }
        close(fd);

        // Add IPC_CREAT flag
        flag |= IPC_CREAT;
    }

    shm->key = ftok(shm->key_path, 1);
    if (shm->key == -1) {
        PRINT_ERR("%s() ftok() : %s\n", __func__, strerror(errno));
        return -3;
    }

    shm->id = shmget(shm->key, shm->mem_size, flag);
    if (shm->id < 0) {
        PRINT_ERR("%s() shmget() : %s\n", __func__, strerror(errno));
        PRINT_ERR("%s() key (%X) size(%d) flag (%X)\n", __func__, shm->key, shm->mem_size, flag);
        return -4;
    }

    shm->mem = shmat(shm->id, NULL, 0);
    if (shm->mem == ((void *)-1)) {
        PRINT_ERR("%s() shmat() : %s\n", __func__, strerror(errno));
        return -5;
    }

    if (flag & IPC_CREAT) {
        // first create memory preiod
        PRINT_DEBUG("%s First CREAT memset to zero\n", __func__);
        memset(shm->mem, 0, shm->mem_size);
    }

    return 0;
}

shm_class *shm_new(void)
{
    shm_class *ins = (shm_class *)omalloc("ShareMemory", sizeof(shm_class));
    if (!ins) { return NULL; }

    memset(ins, 0, sizeof(shm_class));

    ins->init = shm_init;
    ins->destroy = shm_destory;

    return ins;
}
