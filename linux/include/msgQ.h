#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stdint.h>

#define MESSAGE_QUEUE_PATH               "/tmp/messageQ/"
#define MESSAGE_QUEUE_PATH_SIZE          (128)
#define MESSAGE_QUEUE_MAX_SIZE           (2048 - 8)
#define MESSAGE_QUEUE_MAX_CLIENT         (10)

#define MESSAGE_QUEUE_ANY_MESSAGE        (0)
#define MESSAGE_QUEUE_SEND_TO_SERVER     (1)
#define MESSAGE_QUEUE_REG_TO_SERVER      (2)
#define MESSAGE_QUEUE_SERVER_RECV_ALL    (-2)

typedef int (*event_handler)(void *);

#include "timer.h"

typedef struct _msgQ_eve_clt {
    int qid;
    struct timespec timer;
    struct _msgQ_eve_clt *prev;
    struct _msgQ_eve_clt *next;
} msgQ_eve_clt;

typedef struct _msgQ_pkg {
    long msgid;
    long xid;
    uint8_t data[MESSAGE_QUEUE_MAX_SIZE];
} msgQ_pkg;

typedef struct _msgQ_srv {

    // ---- GENERAL METHOD ----
    int (*init)(struct _msgQ_srv *, char *);
    int (*destroy)(struct _msgQ_srv *);

    // ---- PUBLIC METHOD ----
    int (*send)(struct _msgQ_srv *, uint8_t *, size_t);
    int (*recv)(struct _msgQ_srv *, uint8_t *, int, int);

    int (*send_event)(struct _msgQ_srv *, uint8_t *, size_t);
    // may be need used thread or sub process keep running this method
    int (*monitor_request)(struct _msgQ_srv *, int);

    int (*len)(struct _msgQ_srv *);
    int (*clear)(struct _msgQ_srv *);

    int (*add)(struct _msgQ_srv *, msgQ_eve_clt *);
    int (*del)(struct _msgQ_srv *, msgQ_eve_clt *);

    // ---- PRIVATE DATA ----
    
    int qid;
    // this pid is NOT server self pid
    // this var save client pid for response used ( pkg.xid )
    int pid;

    // for event broadcast
    // msgQ_eve_clt *client;
    // int client_amount;
    // msgQ_eve_clt *clt;
    msgQ_eve_clt head;
    int clt_amt;

} msgQ_srv;
msgQ_srv *msgQ_srv_new(void);

/*
} msgQ_eve_clt;
} msgQ_pkg;
msgQ_eve_clt
msgQ_pkg *pkg = (msgQ_pkg *)omalloc("msgQ package", sizeof(msgQ_pkg));
msgQ_srv_new
msgQ_srv *srv = msgQ_srv_new();
msgQ_clt *clt = msgQ_clt_new()
*/

typedef struct _msgQ_clt {

    // ---- GENERAL METHOD ----
    int (*init)(struct _msgQ_clt *, char *);
    int (*destroy)(struct _msgQ_clt *);

    // ---- PUBLIC METHOD ----
    int (*send)(struct _msgQ_clt *, uint8_t *, size_t);
    int (*recv)(struct _msgQ_clt *, uint8_t *, int, int);

    int (*request_event)(struct _msgQ_clt *, char *, event_handler);
    // may be need used thread or sub process keep running this method
    int (*monitor_event)(struct _msgQ_clt *, int);

    // ---- PRIVATE DATA ----
    
    int srvQ;
    int pid;

    int eid;
    event_handler hand;

} msgQ_clt;
msgQ_clt *msgQ_clt_new(void);

#endif
