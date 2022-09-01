#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>

#include "misc.h"
#include "mem.h"
#include "timer.h"
#include "cond.h"

#define CONDITION_NOCHANGED         (0)
#define CONDITION_CHANGED           (1)

int cond_var_init(cond_var *cond)
{
    if (!cond) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    int res = 0;
    res = pthread_condattr_init(&(cond->cond_attr));
    if (res) { PRINT_ERR("%s() pthread_condattr_init failed : %s\n", __func__, strerror(errno)); }
    res = pthread_cond_init(&(cond->cond), &(cond->cond_attr));
    if (res) { PRINT_ERR("%s() pthread_cond_init failed : %s\n", __func__, strerror(errno)); }

    res = pthread_mutexattr_init(&(cond->mux_attr));
    if (res) { PRINT_ERR("%s() pthread_mutexattr_init failed : %s\n", __func__, strerror(errno)); }
    res = pthread_mutex_init(&(cond->mux), &(cond->mux_attr));
    if (res) { PRINT_ERR("%s() pthread_mutex_init failed : %s\n", __func__, strerror(errno)); }

    cond->status = CONDITION_NOCHANGED;

    return 0;
}
int cond_var_destroy(cond_var *cond)
{
    if (!cond) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    int res = 0;
    res = pthread_cond_destroy(&(cond->cond));
    if (res) { PRINT_ERR("%s() pthread_cond_destroy failed : %s\n", __func__, strerror(errno)); }
    res = pthread_mutex_destroy(&(cond->mux));
    if (res) { PRINT_ERR("%s() pthread_mutex_destroy failed : %s\n", __func__, strerror(errno)); }

    free(cond);

    return 0;
}

int cond_var_changed(cond_var *cond)
{
    if (!cond) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // lock
    pthread_mutex_lock(&(cond->mux));
    // change status
    cond->status = CONDITION_CHANGED;
    // unlock
    pthread_mutex_unlock(&(cond->mux));
    // signal
    pthread_cond_signal(&(cond->cond));

    return 0;
}
int cond_var_broadcast(cond_var *cond)
{
    if (!cond) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // lock
    pthread_mutex_lock(&(cond->mux));
    // change status
    cond->status = CONDITION_CHANGED;
    // unlock
    pthread_mutex_unlock(&(cond->mux));
    // signal
    pthread_cond_broadcast(&(cond->cond));

    return 0;
}
int cond_var_wait(cond_var *cond)
{
    if (!cond) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // lock
    pthread_mutex_lock(&(cond->mux));
    // wait
    pthread_cond_wait(&(cond->cond), &(cond->mux));
    // unlock
    pthread_mutex_unlock(&(cond->mux));

    return 0;
}
int cond_var_timewait(cond_var *cond, int ms)
{
    if (!cond) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // lock
    pthread_mutex_lock(&(cond->mux));

    // wait
    Timer tout;
    tm_set_ms(&tout, ms);

    int res = pthread_cond_timedwait(&(cond->cond), &(cond->mux), &tout);
    // Lock Timeout
    if (res == ETIMEDOUT) { return 1; }
    else { return 0; }

    // unlock
    pthread_mutex_unlock(&(cond->mux));

    return 0;
}

cond_var *cond_var_new(void)
{
    cond_var *cond = (cond_var *)omalloc("condition variable", sizeof(cond_var));
    if (!cond) {
        PRINT_ERR("%s() omalloc failed\n", __func__);
        return NULL;
    }

    cond->init = cond_var_init;
    cond->destroy = cond_var_destroy;

    cond->changed = cond_var_changed;
    cond->broadcast = cond_var_broadcast;
    cond->wait = cond_var_wait;
    cond->timewait = cond_var_timewait;

    return cond;
}
