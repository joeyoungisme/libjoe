#ifndef _MISC_CONDITION_VARIABLE_H
#define _MISC_CONDITION_VARIABLE_H

#define CONDITION_NOCHANGED         (0)
#define CONDITION_CHANGED           (1)

typedef struct _cond_var {

    // ---- GENERAL METHOD ----
    int (*init)(struct _cond_var *);
    int (*destroy)(struct _cond_var *);

    // ---- PUBLIC METHOD ----
    int (*changed)(struct _cond_var *);
    int (*broadcast)(struct _cond_var *);
    int (*wait)(struct _cond_var *);
    int (*timewait)(struct _cond_var *, int);

    // ---- PRIVATE METHOD ----

    // ---- PRIVATE DATA ----
    pthread_mutexattr_t mux_attr;
    pthread_mutex_t mux;
    pthread_condattr_t cond_attr;
    pthread_cond_t cond;

    unsigned char status;
} cond_var;

cond_var *cond_var_new(void);

#endif
