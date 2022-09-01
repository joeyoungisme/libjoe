#ifndef MISC_SOCKET_H
#define MISC_SOCKET_H

typedef enum {
    SOCKET_TCP,
    SOCKET_UDP,
    SOCKET_DOMAIN,
    SOCKET_TYPE_AMOUNT
} SOCKET_TYPE;

typedef struct _sock_client {
    // ---- GENERAL METHOD ----
    int (*init)(struct _sock_client *, SOCKET_TYPE);
    int (*destroy)(struct _sock_client *);

    // ---- PUBLIC METHOD ----
    // ---- PRIVATE METHOD ----
    // ---- PRIVATE MEMBER ----
} sock_client;

sock_client *sock_client_new(void);


typedef struct _sock_server {
    // ---- GENERAL METHOD ----
    int (*init)(struct _sock_server *, SOCKET_TYPE);
    int (*destroy)(struct _sock_server *);

    // ---- PUBLIC METHOD ----
    // ---- PRIVATE METHOD ----
    // ---- PRIVATE MEMBER ----
} sock_server;


sock_server *sock_server_new(void);


#endif
