#ifndef MISC_LINKINGLIST_H
#define MISC_LINKINGLIST_H

#include "mutex.h"

#define INIT_LIST_HEADER          NULL

typedef struct __list_node {
    void *ins;
    struct __list_node *next;
    struct __list_node *prev;
} list_node;

struct __list_header {
    list_node *first;
    list_node *tail;
    unsigned int node_cnt;
};

typedef struct __list_header * list_header;

int list_header_init(list_header *);
int list_header_delete(list_header *);
int list_header_length(list_header);

int list_insert_tail(list_header head, void *ins);
int list_insert_index(list_header head, void *ins, unsigned int idx);
int list_insert_first(list_header head, void *ins);

void *list_get_tail(list_header head);
void *list_get_index(list_header head, unsigned int idx);
void *list_get_first(list_header head);

int list_remove_tail(list_header head);
int list_remove_index(list_header head, unsigned int idx);
int list_remove_first(list_header head);

typedef struct _linklist {

    // ---- PUBLIC METHOD ----

    int (*init)(struct _linklist *);
    int (*destroy)(struct _linklist *);
    int (*len)(struct _linklist *);
    int (*lock)(struct _linklist *);
    int (*unlock)(struct _linklist *);

    int (*add_first)(struct _linklist *, void *);
    int (*add_end)(struct _linklist *, void *);
    int (*add_middle)(struct _linklist *, int idx, void *);

    void *(*get_first)(struct _linklist *);
    void *(*get_end)(struct _linklist *);
    void *(*get_middle)(struct _linklist *, int);

    int (*rm_first)(struct _linklist *);
    int (*rm_end)(struct _linklist *);
    int (*rm_middle)(struct _linklist *, int);

    // ---- PRIVATE DATA ----

    struct __list_header head;

    mutex_class *mutex;

} linklist;
linklist *LinklistNew(void);

#endif

