#ifndef MISC_SHARE_MEMORY
#define MISC_SHARE_MEMORY

#include <sys/ipc.h>

#define SHM_KEY_PATH_LEN        128

typedef struct _shm_class {
    // public
    int (*init)(struct _shm_class *, char *, unsigned int);
    int (*destroy)(struct _shm_class *);


    // private attr
    int id;
    key_t key;
    void *mem;
    unsigned int mem_size;
    char key_path[SHM_KEY_PATH_LEN];
} shm_class;

int shm_init(shm_class *, char *, unsigned int);
shm_class *shm_new(void);
void shm_free(shm_class *);

#endif
