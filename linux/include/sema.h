#ifndef MISC_SEMA_MANAGER_
#define MISC_SEMA_MANAGER_

#define SEMA_KEY_PATH_LEN        128

typedef enum {
    SEMA_INIT_FORCE_LOCK,
    SEMA_INIT_FORCE_UNLOCK,
    SEMA_INIT_EXCEP_LOCK,
    SEMA_INIT_EXCEP_UNLOCK,
    SEMA_INIT_AMOUNT
} SEMA_INIT_STATE;

union semnu {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

typedef struct _sema_class {
    // public
    int (*init)(struct _sema_class *, char *, SEMA_INIT_STATE);
    int (*destroy)(struct _sema_class *);

    int (*lock)(struct _sema_class *);
    int (*tmlock)(struct _sema_class *, unsigned int);
    int (*unlock)(struct _sema_class *);

    // private attr
    int id;
    key_t key;
    char key_path[SEMA_KEY_PATH_LEN];
    union semnu arg;
} sema_class;

sema_class *sema_new(void);

#endif
