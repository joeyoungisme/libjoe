#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// for open()
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/sem.h>

#include "mem.h"
#include "misc.h"
#include "sema.h"
#include "timer.h"

static int sema_timelock(sema_class *sema, unsigned int ms)
{
    if (!sema) {
        PRINT_ERR("%s() invalid args \n", __func__);
        return -1;
    }

    // Sema Lock
    struct sembuf semset = {
        .sem_num = 0,
        .sem_op = -1,
        .sem_flg = IPC_NOWAIT
    };

    Timer tmr = {0};
    tm_set_ms(&tmr, ms);

    while(semop(sema->id, &semset, 1) < 0)
    {
        if (errno == EINTR)
        {
            PRINT_ERR("%s semop() : %s continue\n", __func__, strerror(errno));
            continue;
        } else if (errno == EAGAIN) {

            if (tm_is_timeout(tmr)) {
                PRINT_ERR("%s() tmlock failed ( %d ms ).\n", __func__, ms);
                return -1;
            }

            usleep(1000);
            continue;
        }

        PRINT_ERR("%s semop() : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}
static int sema_lock(sema_class *sema)
{
    if (!sema) {
        PRINT_ERR("%s() invalid args \n", __func__);
        return -1;
    }

    // Sema Lock
    struct sembuf semset = {
        .sem_num = 0,
        .sem_op = -1,
        .sem_flg = 0
    };

    while(semop(sema->id, &semset, 1) < 0)
    {
        if(errno == EINTR)
        {
            PRINT_ERR("%s semop() : %s continue\n", __func__, strerror(errno));
            continue;
        }

        PRINT_ERR("%s semop() : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}
static int sema_unlock(sema_class *sema)
{
    if (!sema) {
        PRINT_ERR("%s() invalid args \n", __func__);
        return -1;
    }

    // Sema Unlock
    struct sembuf semset = {
        .sem_num = 0,
        .sem_op = 1,
        .sem_flg = 0
    };

    while(semop(sema->id, &semset, 1) < 0)
    {
        if(errno == EINTR)
        {
            PRINT_ERR("%s semop() : %s continue\n", __func__, strerror(errno));
            continue;
        }

        PRINT_ERR("%s semop() : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * val : sema init value , normally set 1 ( we need first lock and unlock last )
 *       if init value set to 0 , when we call lock() function first time, we will be blocked
 */
static int sema_init(sema_class *sema, char *path, SEMA_INIT_STATE init)
{
    if (!sema || !path) {
        PRINT_ERR("%s() Invalid arg\n", __func__);
        return -1;
    }

    memset(sema->key_path, 0, SEMA_KEY_PATH_LEN);
    snprintf(sema->key_path, SEMA_KEY_PATH_LEN, "%s", path);

    int flag = 0666;

    // check specific key file exist
    if (access(sema->key_path, F_OK)) {

        // create new key file
        int fd = open(sema->key_path, (O_CREAT | O_RDWR), 0777);
        if (fd < 0) {
            PRINT_ERR("%s() open() : %s\n", __func__, strerror(errno));
            return -2;
        }
        close(fd);

        // Add IPC_CREAT flag
        flag |= IPC_CREAT;
    }

    sema->key = ftok(sema->key_path, 1);
    if (sema->key == -1) {
        PRINT_ERR("%s() ftok() : %s\n", __func__, strerror(errno));
        return -3;
    }

    sema->id = semget(sema->key, 1, flag);
    if (sema->id < 0) {
        PRINT_ERR("%s() semget() : %s\n", __func__, strerror(errno));
        return -4;
    }

    if (init == SEMA_INIT_FORCE_LOCK) {
        sema->arg.val = 0;
        if (semctl(sema->id, 0, SETVAL, sema->arg) < 0) {
            PRINT_ERR("%s() semctl : %s\n", __func__, strerror(errno));
            return -5;
        }
    } else if (init == SEMA_INIT_FORCE_UNLOCK) {
        sema->arg.val = 1;
        if (semctl(sema->id, 0, SETVAL, sema->arg) < 0) {
            PRINT_ERR("%s() semctl : %s\n", __func__, strerror(errno));
            return -5;
        }
    } else if (init == SEMA_INIT_EXCEP_LOCK) {
        if (flag & IPC_CREAT) {
            sema->arg.val = 0;
            if (semctl(sema->id, 0, SETVAL, sema->arg) < 0) {
                PRINT_ERR("%s() semctl : %s\n", __func__, strerror(errno));
                return -5;
            }
        }
    } else if (init == SEMA_INIT_EXCEP_UNLOCK) {
        if (flag & IPC_CREAT) {
            sema->arg.val = 1;
            if (semctl(sema->id, 0, SETVAL, sema->arg) < 0) {
                PRINT_ERR("%s() semctl : %s\n", __func__, strerror(errno));
                return -5;
            }
        }
    }

    return 0;
}

static int sema_destroy(sema_class *sema)
{
    if (!sema) {
        PRINT_ERR("%s() Invalid arg\n", __func__);
        return -1;
    }

    // Remove key file ??
    ofree(sema);

    return 0;
}

sema_class *sema_new(void)
{
    sema_class *ins = (sema_class *)omalloc("sema obj", sizeof(sema_class));
    if (!ins) { return NULL; }

    memset(ins, 0, sizeof(sema_class));

    ins->init = sema_init;
    ins->destroy = sema_destroy;
    ins->lock = sema_lock;
    ins->tmlock = sema_timelock;
    ins->unlock = sema_unlock;

    return ins;
}
