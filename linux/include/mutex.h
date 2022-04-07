#ifndef MISC_MUTEX_H
#define MISC_MUTEX_H

#include <pthread.h>

typedef struct _mutex_class {
    // public
    int (*init)(struct _mutex_class *);
    int (*destroy)(struct _mutex_class *);

    int (*lock)(struct _mutex_class *);
    int (*tmlock)(struct _mutex_class *, unsigned int);
    int (*unlock)(struct _mutex_class *);

    // ---- PRIVATE DATA ----

    pthread_mutex_t mux;

} mutex_class;

mutex_class *mutex_new(void);

#endif
