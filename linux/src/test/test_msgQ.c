#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "misc.h"
#include "mem.h"
#include "timer.h"
#include "msgQ.h"

static int aflag = 0;
static int bflag = 0;
static char key[64] = {0};

int clt_event_handler(void *data)
{
    PRINT_DEBUG("client (%d) recv event : %s\n", getpid(), (char *)data);
    return 0;
}

void msgQ_client(void)
{
    // client
    msgQ_clt *clt = msgQ_clt_new();
    if (!clt) {
        PRINT_ERR("msgQ_clt_new failed\n");
        exit(EXIT_FAILURE);
    } else if (clt->init(clt, key)) {
        PRINT_ERR("msgQ clt init failed\n");
        exit(EXIT_FAILURE);
    }

    struct timespec ask_timer = {0};
    tm_set_ms(&ask_timer, 5000);

    while(1)
    {
        if (bflag) {
            PRINT_DEBUG("----- Broadcast -----\n");
            // monitor
            if (!clt->request_event(clt, "testQ", clt_event_handler)) {
                int moni_res = clt->monitor_event(clt, 500);
                if (moni_res > 0) {
                    // PRINT_DEBUG("client monitor timeout\n");
                } else if (moni_res < 0) {
                    PRINT_ERR("client monitor failed\n");
                } else {
                    // PRINT_DEBUG("automatically execute callback.\n");
                }
            } else {
                PRINT_ERR("request_event failed.\n");
            }
            PRINT_DEBUG("---------------------\n");
        }


        if (!tm_is_timeout(ask_timer)) { continue; }

        tm_set_ms(&ask_timer, 5000);

        if (aflag) {

            PRINT_DEBUG("----- Ask/Ack -----\n");
            // Send Ask 
            if (clt->send(clt, "Ask", 3)) {
                PRINT_ERR("msgQ client send failed\n");
            }

            // Recv Ack
            char rbuff[128] = {0};
            int res = clt->recv(clt, rbuff, 128, 5000);
            if (res > 0) {
                // PRINT_DEBUG("msgQ client recv timeout\n");
            } else if (res < 0) {
                PRINT_ERR("msgQ client recv failed\n");
            } else {
                PRINT_DEBUG("msgQ client (%d) recv : %s\n", getpid(), rbuff);
            }

            PRINT_DEBUG("-------------------\n");
        }
    }
}

void msgQ_server(void)
{
    // server
    msgQ_srv *srv = msgQ_srv_new();
    if (!srv) {
        PRINT_ERR("msgQ_srv_new failed\n");
        exit(EXIT_FAILURE);
    } else if (srv->init(srv, key)) {
        PRINT_ERR("msgQ srv init failed\n");
        exit(EXIT_FAILURE);
    }

    //  int (*send_event)(struct _msgQ_srv *, uint8_t *, size_t);
    //  int (*monitor_event)(struct _msgQ_srv *, int);

    struct timespec event_timer = {0};
    tm_set_ms(&event_timer, 5000);

    while(1)
    {
        if (aflag) {
            PRINT_DEBUG("----- Ask/Ack -----\n");
            // Recv Ask
            char rbuff[128] = {0};
            int recv_res = srv->recv(srv, rbuff, 128, 500);
            if (recv_res > 0) {
                // PRINT_DEBUG("msgQ server recv timeout\n");
            } else if (recv_res < 0) {
                PRINT_ERR("msgQ server recv failed\n");
            } else {
                PRINT_DEBUG("msgQ server (%d) recv : %s\n", getpid(), rbuff);

                // Send Ack
                if (srv->send(srv, "Ack", 3)) {
                    PRINT_ERR("msgQ server send failed\n");
                }
            }
            PRINT_DEBUG("-------------------\n");
        }

        if (bflag) {
            PRINT_DEBUG("----- Broadcast -----\n");
            // monitor 500ms
            int moni_res = srv->monitor_request(srv, 500);
            if (moni_res > 0) {
                // PRINT_DEBUG("msgQ server monitor timeout\n");
            } else if (moni_res < 0) {
                PRINT_ERR("msgQ server monitor failed\n");
            } else {
                // show client list
                msgQ_eve_clt *clt = srv->head.next;
                while (clt != &(srv->head))
                {
                    PRINT_DEBUG("clt Q (%d) : alive %d ms\n", clt->qid, tm_stopwatch(clt->timer));
                    clt = clt->next;
                }
            }

            if (tm_is_timeout(event_timer)) {
                char eve_str[] = "!!!! Server Broadcast !!!!";
                PRINT_DEBUG("server send event : %s\n", eve_str);
                srv->send_event(srv, eve_str, strlen(eve_str));
                tm_set_ms(&event_timer, 5000);
            }
            PRINT_DEBUG("---------------------\n");
        }
    }
}

int main(int argc, char *argv[])
{
    int role = 0;

    int opt = 0;
    while((opt = getopt(argc, argv, "absck:")) != -1)
    {
        switch(opt)
        {
            case 'a':
                aflag = 1;
                break;
            case 'b':
                bflag = 1;
                break;
            case 's':
                role = 1;
                break;
            case 'c':
                role = 0;
                break;
            case 'k':
                snprintf(key, 64, "%s", optarg);
                break;
            default:
                printf("usage : %s [-c|-s] [-a] [-b] [-k /tmp/test.key]\nc : client\ns : server\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!strlen(key)) {
        printf("usage : %s [-c|-s] [-a] [-b] [-k /tmp/test.key]\nc : client\ns : server\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (role) {
        msgQ_server();
    } else {
        msgQ_client();
    }

    exit(EXIT_SUCCESS);
}
