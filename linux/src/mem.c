#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include "misc.h"
#include "mem.h"

struct mem_log {
    char dec[64];
    void *addr;
    size_t size;
};

static struct mem_log mem_manager[MEMORY_ALLOCATE_MAX_TIMES] = {0};

void *omalloc(char *dec, size_t size)
{
    if (!dec || !size) { return NULL; }

    for (int idx = 0; idx < MEMORY_ALLOCATE_MAX_TIMES; ++idx) {

        if (mem_manager[idx].addr) { continue; }

        void *space = malloc(size);
        if (!space) {
            PRINT_ERR("%s() malloc failed : %s\n", __func__, strerror(errno));
            return space;
        }

        time_t now;
        time(&now);
        snprintf(mem_manager[idx].dec, 64, "[%ld] %s", now, dec);
        mem_manager[idx].addr = space;
        mem_manager[idx].size = size;

        return space;
    }

    PRINT_ERR("%s() too many space allocate, please free some space first\n", __func__);
    return NULL;
}

int ofree(void *addr)
{
    if (!addr) { return -1; }

    for (int idx = 0; idx < MEMORY_ALLOCATE_MAX_TIMES; ++idx) {
        if (!mem_manager[idx].addr) { continue; }
        else if (mem_manager[idx].addr != addr) { continue; }

        free(mem_manager[idx].addr);
        memset(&mem_manager[idx], 0, sizeof(struct mem_log));

        return 0;
    }

    PRINT_ERR("%s() invalid memory address.\n", __func__);
    PRINT_ERR("WARNING : be careful memory leak.\n");
    return -1;
}

void mem_show(void)
{
    printf(" --------- %s ---------\n", __func__);
    for (int idx = 0; idx < MEMORY_ALLOCATE_MAX_TIMES; ++idx) {
        if (!mem_manager[idx].addr) { continue; }

        printf("%-32s : 0x%p ( %ld Byte )\n", mem_manager[idx].dec, mem_manager[idx].addr, mem_manager[idx].size);
    }
    printf(" ----------------------\n");
}
