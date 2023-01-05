#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "memory_manager.h"

#define BUFFER_SIZE                      (1024 + 512)
static uint8_t share_space[BUFFER_SIZE] = {0};
static memory_manager mmr_inst = {0};
static memory_manager * const mmr = &mmr_inst;

#define RECORD_SIZE                      (10)

void *point_record[RECORD_SIZE] = {0};

void allocate_process(memory_manager *mmr)
{
    for (int idx = 0; idx < RECORD_SIZE; ++idx)
    {
        printf("[Allocate Mode]");
        int rand_size = (rand() % 200) + 50;
        printf(" [ left %04ld byte ] - [ %03d byte ]", mmr->memory_left(mmr), rand_size);

        point_record[idx] = mmr->allocate(mmr, rand_size);
        if (!point_record[idx]) {
            printf(" mmr->allocate failed ( left %ld byte )\n", mmr->memory_left(mmr));
        } else {
            printf(" [%d] success : %p.\n", idx, point_record[idx]);
        }
    }

    return;
}

void free_process(memory_manager *mmr)
{
    for (int idx = 0; idx < RECORD_SIZE; ++idx)
    {
        printf("[Free Mode]");
        if (!point_record[idx]) {
            printf(" [%d] NULL\n", idx);
        } else {

            if (mmr->free(mmr, point_record[idx])) {
                printf("mmr->free failed\n");
            } else {
                printf(" [%d] success : %p.\n", idx, point_record[idx]);
                point_record[idx] = NULL;
            }
        }
    }

    return;
}

int main(int argc, char *argv[])
{
    // init
    if (memory_manager_static_init(mmr, share_space, BUFFER_SIZE)) {
        printf("memory_manager_static_init failed\n");
        exit(EXIT_FAILURE);
    } else {
        printf("memory_manager_static_init success\n");
    }

    time_t seed;
    srand((unsigned) time(&seed));

    while(1)
    {
        printf(" ------------ \n");
        allocate_process(mmr);
        free_process(mmr);

        usleep(200000);
    }

    return 0;
}
