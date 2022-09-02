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

#ifndef PRINT_ERR
#define PRINT_ERR(...)
#endif

#ifndef PRINT_DEBUG
#define PRINT_DEBUG(...)
#endif

static int msgQ_get_qid(char *name)
{
    if (!name) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    struct stat st = {0};
    if (stat(MESSAGE_QUEUE_PATH, &st) == -1) {
        if (mkdir(MESSAGE_QUEUE_PATH, 0777) == -1) {
            PRINT_ERR("%s() mkdir %s failed\n", __func__, MESSAGE_QUEUE_PATH);
            return -1;
        }
    }

    char path[MESSAGE_QUEUE_PATH_SIZE] = {0};
    memset(path, 0, MESSAGE_QUEUE_PATH_SIZE);
    snprintf(path, MESSAGE_QUEUE_PATH_SIZE, "%s/%s", MESSAGE_QUEUE_PATH, name);

    // if key file not exist , touch new file
    if (access(path, F_OK)) {
        int fd = open(path, (O_CREAT | O_RDWR), 0777);
        if (fd < 0) {
            PRINT_ERR("%s() open %s (CREAT) failed : %s\n", __func__, path, strerror(errno));
            return -1;
        }
        close(fd);
    }

    key_t key = ftok(path, 1);
    if (key == -1) {
        fprintf(stderr, "ftok() : %s\n", strerror(errno));
        return -1;
    }

    int retQ = msgget(key, 0777);
    if (retQ == -1) {
        if (errno != ENOENT) {
            printf("msgget() : %s\n", strerror(errno));
            return -1;
        }

        retQ = msgget(key, IPC_CREAT|0777);
        if (retQ == -1) {
            printf("msgget() : %s\n", strerror(errno));
            return -1;
        }
    }

    return retQ;
}

int msgQ_clt_send(msgQ_clt *client, uint8_t *data, size_t len)
{
    if (!client || !data || !len) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    } else if (len > MESSAGE_QUEUE_MAX_SIZE) {
        PRINT_ERR("%s() too many data size ... \n", __func__);
        return -1;
    }

    msgQ_pkg pkg = {
        .msgid = MESSAGE_QUEUE_SEND_TO_SERVER,
        .xid = getpid(),
    };

    memcpy(pkg.data, data, len);

    while (1)
    {
        if (!msgsnd(client->srvQ, &pkg, len + sizeof(pkg.xid), 0)) { break; }

        if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN) {
            PRINT_ERR("%s() Queue %d is full (EAGAIN) \n", __func__, client->srvQ);
            return -1;
        } else {
            PRINT_ERR("%s() Queue %d : %s \n", __func__, client->srvQ, strerror(errno));
            return -1;
        }
    }

    return 0;
}
int msgQ_clt_recv(msgQ_clt *client, uint8_t *data, int len, int tout)
{
    if (!client || !data || !len) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    int flag = 0;
    Timer timer = {0};
    if (tout) {
        flag |= IPC_NOWAIT;
        tm_set_ms(&timer, tout);
    }

    msgQ_pkg pkg = {0};

    int cnt = 0;
    while (1)
    {
        cnt = msgrcv(client->srvQ, &pkg, MESSAGE_QUEUE_MAX_SIZE, getpid(), flag);

        if (cnt >= 0) {
            break;
        } else if (tm_is_timeout(timer)) {
            // PRINT_ERR("%s() msgrcv timeout \n", __func__);
            return 1;
        } else if (errno != ENOMSG) {

            if (errno == EIDRM) {
                PRINT_ERR("%s() client self queue remove ?\n", __func__);
            } else {
                PRINT_ERR("%s() msgrcv error : %s\n", __func__, strerror(errno));
            }

            return -1;
        }
    }

    cnt -= sizeof(pkg.xid);

    if (cnt != len) {
        PRINT_ERR("%s() WARNING : srv want %d byte, but get %d byte.\n", __func__, len, cnt);
    }
    if (cnt > len) {
        PRINT_ERR("%s() WARNING : (get > want) may cause data loss.\n", __func__);
        cnt = len;
    }

    memcpy(data, pkg.data, cnt);

    return 0;
}
int msgQ_clt_monitorevent(msgQ_clt *client, int tout)
{
    if (!client) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    int flag = 0;
    Timer timer = {0};
    if (tout) {
        flag |= IPC_NOWAIT;
        tm_set_ms(&timer, tout);
    }

    msgQ_pkg pkg = {0};

    static int tout_cnt = 0;

    while (1)
    {
        int res = msgrcv(client->eid, &pkg, MESSAGE_QUEUE_MAX_SIZE, MESSAGE_QUEUE_ANY_MESSAGE, flag);

        if (res >= 0) {
            break;
        } else if (tm_is_timeout(timer)) {
            if (tout_cnt++ > 10) {
                PRINT_ERR("%s() client timeout too many times.\n", __func__);
                PRINT_ERR("%s() client request event again.\n", __func__);
                client->pid = 0;
                client->hand = NULL;
                tout_cnt = 0;
                return -1;
            }
            return 1;
        } else if (errno != ENOMSG) {

            if (errno == EIDRM) {
                PRINT_ERR("%s() client self queue remove ? (%s)\n", __func__, strerror(errno));
            }

            client->pid = 0;
            client->hand = NULL;
            return -1;

        }

        // delay 1ms
        usleep(1000);
    }

    if (!client->hand) {
        PRINT_ERR("%s() WARNING get event but have not handler ...\n", __func__);
        return -1;
    }

    if (client->hand(pkg.data)) {
        PRINT_ERR("%s() call back handler error\n", __func__);
        return -1;
    }

    return 0;
}
int msgQ_clt_requestevent(msgQ_clt *client, char *key, event_handler hand)
{
    if (!client || !key || !hand) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (client->pid) {
        // represent already get server pid. change handler only
        client->hand = hand;
        return 0;
    }

    // new client private Q
    char ename[64] = {0};
    snprintf(ename, 64, "msgQ_client_%s.key", key);
    client->eid = msgQ_get_qid(ename);
    if (client->eid < 0) {
        PRINT_ERR("%s() private event id get failed\n", __func__);
        return -1;
    }

    // send reg package, inclding private Qid
    msgQ_pkg pkg = {
        .msgid = MESSAGE_QUEUE_REG_TO_SERVER,
        .xid = client->eid,
    };

    while (1)
    {
        if (!msgsnd(client->srvQ, &pkg, sizeof(pkg.xid), IPC_NOWAIT)) { break; }

        if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN) {
            PRINT_ERR("%s() Queue %d is full (EAGAIN) \n", __func__, client->srvQ);
            msgctl(client->eid, IPC_RMID, NULL);
            return -1;
        } else {
            PRINT_ERR("%s() Queue %d : %s \n", __func__, client->srvQ, strerror(errno));
            msgctl(client->eid, IPC_RMID, NULL);
            return -1;
        }
    }


    Timer timer = {0};
    tm_set_ms(&timer, 3000);

    memset(&pkg, 0, sizeof(msgQ_pkg));

    while (1)
    {
        int res = msgrcv(client->eid, &pkg, MESSAGE_QUEUE_MAX_SIZE, MESSAGE_QUEUE_ANY_MESSAGE, IPC_NOWAIT);
        if (res >= 0) {
            client->pid = pkg.xid;
            PRINT_DEBUG("%s() event register success.\n", __func__);
            break;
        } else if (tm_is_timeout(timer)) {
            PRINT_ERR("%s() event register timeout \n", __func__);
            msgctl(client->eid, IPC_RMID, NULL);
            return -1;
        } else if (errno != ENOMSG) {
            PRINT_ERR("%s() msgrcv error : %s\n", __func__, strerror(errno));
            msgctl(client->eid, IPC_RMID, NULL);
            return -1;
        }

        usleep(1000);
    }

    // save handler point
    client->hand = hand;

    return 0;
}
int msgQ_clt_destroy(msgQ_clt *client)
{
    if (!client) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (msgctl(client->srvQ, IPC_RMID, NULL)) {
        PRINT_ERR("%s() msgctl : %s\n", __func__, strerror(errno));
        return -1;
    }

    ofree(client);

    return 0;
}
int msgQ_clt_init(msgQ_clt *client, char *name)
{
    if (!client || !name) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    srand(time(NULL));

    client->srvQ = msgQ_get_qid(name);
    if (client->srvQ < 0) {
        PRINT_ERR("%s() Get Queue Id failed\n", __func__);
        return -1;
    }

    return 0;
}
msgQ_clt *msgQ_clt_new(void)
{
    msgQ_clt *client = (msgQ_clt *)omalloc("msgQ_clt", sizeof(msgQ_clt));
    if (!client) {
        PRINT_ERR("%s() omalloc failed\n", __func__);
        return NULL;
    }
    memset(client, 0, sizeof(msgQ_clt));

    client->init = msgQ_clt_init;
    client->destroy = msgQ_clt_destroy;

    client->send = msgQ_clt_send;
    client->recv = msgQ_clt_recv;

    client->request_event = msgQ_clt_requestevent;
    client->monitor_event = msgQ_clt_monitorevent;

    return client;
}

/* 
 *         .__________.
 *         v          |
 * server->head.prev -.
 *         ^   .next -.
 *         |__________|
 *
 *         ._______________.
 *         |               |
 *         v               |
 * server->head.prev \_    |
 *         ^   .next / |   |
 *         |           |   |
 *         |           v_  |
 *         |          |__| |
 *         .___________/\__/
 *
 *         ._____________________________________.
 *         |                                     |
 *         v                                     |
 * server->head.prev ----------------------.     |
 *         ^   .next -.                    |     |
 *         |          |                    |     |
 *         |          v____________________v_    |
 *         |         |__|__|__|__|__|__|__|__|   |
 *         .__________/\_/\_/\_/\_/\_/\_/\_/\____/
 */

int msgQ_srv_add(msgQ_srv *srv, msgQ_eve_clt *clt)
{
    if (!srv || !clt) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (clt == &(srv->head)) {
        PRINT_ERR("%s() invalid client\n", __func__);
        return -1;
    }

    clt->prev = (srv->head.prev);
    clt->next = &(srv->head);
    (srv->head.prev)->next = clt;
    (srv->head.prev) = clt;

    srv->clt_amt++;

    return 0;
}
int msgQ_srv_del(msgQ_srv *server, msgQ_eve_clt *del_client)
{
    if (!server || !del_client) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (del_client == &(server->head)) {
        PRINT_ERR("%s() invalid client\n", __func__);
        return -1;
    }

    msgQ_eve_clt *client = server->head.next;
    while(client != &(server->head))
    {
        if (client == del_client) {

            (client->prev)->next = client->next;
            (client->next)->prev = client->prev;

            server->clt_amt--;

            ofree(client);

            return 0;
        }

        client = client->next;
    }

    PRINT_ERR("%s() client %p not found.\n", __func__, del_client);
    return -1;
}

int msgQ_srv_length(msgQ_srv *server)
{
    if (!server) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    struct msqid_ds info = {0};

    if (msgctl(server->qid, IPC_STAT, &info)) {
        PRINT_ERR("%s() msgctl error : %s\n", __func__, strerror(errno));
        return -1;
    }

    return info.msg_qnum;
}
int msgQ_srv_clear(msgQ_srv *server)
{
    if (!server) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    } else if (!server->len(server)) {
        return 0;
    }

    while(server->len(server))
    {
        msgQ_pkg pkg = {0};
        msgrcv(server->qid, &pkg, MESSAGE_QUEUE_MAX_SIZE, 0, IPC_NOWAIT);
    }

    return 0;
}
int msgQ_srv_send(msgQ_srv *server, uint8_t *data, size_t len)
{
    if (!server || !data || !len) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    } else if (len > MESSAGE_QUEUE_MAX_SIZE) {
        PRINT_ERR("%s() too many data size ... \n", __func__);
        return -1;
    } else if (!server->pid) {
        PRINT_ERR("%s() have not destination \n", __func__);
        return -1;
    }

    msgQ_pkg pkg = {
        .msgid = server->pid,
    };

    memcpy(pkg.data, data, len);
    server->pid = 0;

    while (1)
    {
        if (!msgsnd(server->qid, &pkg, len + sizeof(pkg.xid), IPC_NOWAIT)) { break; }

        if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN) {
            PRINT_ERR("%s() Queue %d is full (EAGAIN) \n", __func__, server->qid);
            return -1;
        } else {
            PRINT_ERR("%s() Queue %d : %s \n", __func__, server->qid, strerror(errno));
            return -1;
        }
    }

    return 0;
}
int msgQ_srv_recv(msgQ_srv *server, uint8_t *data, int len, int tout)
{
    if (!server || !data || !len) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }
    if (server->pid) {
        PRINT_DEBUG("%s() server recv busy ( pid already exist )\n", __func__);
        return 0;
    }

    int flag = 0;
    Timer timer = {0};
    if (tout) {
        flag |= IPC_NOWAIT;
        tm_set_ms(&timer, tout);
    }

    msgQ_pkg pkg = {0};

    int cnt = 0;

    while (1)
    {
        cnt = msgrcv(server->qid, &pkg, MESSAGE_QUEUE_MAX_SIZE, MESSAGE_QUEUE_SEND_TO_SERVER, flag);

        if (cnt >= 0) {
            break;
        } else if (tm_is_timeout(timer)) {
            // PRINT_ERR("%s() msgrcv timeout !!\n", __func__);
            return 1;
        } else if (errno != ENOMSG) {

            if (errno == EIDRM) {
                PRINT_ERR("%s() server self queue remove ? (%s)\n", __func__, strerror(errno));
            } else {
                PRINT_ERR("%s() msgrcv error : %s\n", __func__, strerror(errno));
            }

            return -1;
        }

        usleep(1000);
    }

    server->pid = pkg.xid;
    cnt -= sizeof(pkg.xid);

    if (cnt != len) {
        PRINT_ERR("%s() WARNING : srv want %d byte, but get %d byte.\n", __func__, len, cnt);
    }
    if (cnt > len) {
        PRINT_ERR("%s() WARNING : (get > want) may cause data loss.\n", __func__);
        cnt = len;
    }

    memcpy(data, pkg.data, cnt);

    return 0;
}
int msgQ_srv_sendevent(msgQ_srv *server, uint8_t *data, size_t len)
{
    if (!server || !data || !len) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    } else if (len > MESSAGE_QUEUE_MAX_SIZE) {
        PRINT_ERR("%s() too many data size ... \n", __func__);
        return -1;
    }

    msgQ_pkg pkg = {
        .msgid = getpid(),
        .xid = 0,
    };

    memcpy(pkg.data, data, len);

    msgQ_eve_clt *client = server->head.next;
    while (client != &(server->head))
    {
        PRINT_DEBUG("%s() SEND EVENT TO (%d)  \n", __func__, client->qid);
        if (msgsnd(client->qid, &pkg, len + sizeof(pkg.xid), IPC_NOWAIT)) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                PRINT_ERR("%s() Queue %d is full (EAGAIN) \n", __func__, client->qid);
            } else {
                // Remove this client in queue
                PRINT_ERR("%s() Queue %d : %s \n", __func__, client->qid, strerror(errno));
                PRINT_ERR("%s() remove queue %d.\n", __func__, client->qid);
                server->del(server, client);
            }
        }

        client = client->next;
    }

    return 0;
}
int msgQ_srv_monitorrequest(msgQ_srv *server, int tout)
{
    if (!server) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    int flag = 0;
    Timer timer = {0};
    if (tout) {
        flag |= IPC_NOWAIT;
        tm_set_ms(&timer, tout);
    }

    msgQ_pkg pkg = {0};

    while (1)
    {
        int res = msgrcv(server->qid, &pkg, MESSAGE_QUEUE_MAX_SIZE, MESSAGE_QUEUE_REG_TO_SERVER, flag);

        if (res >= 0) {
            break;
        } else if (tm_is_timeout(timer)) {
            // PRINT_ERR("%s() msgrcv timeout !!\n", __func__);
            return 1;
        } else if (errno != ENOMSG) {

            if (errno == EIDRM) {
                PRINT_ERR("%s() server self queue remove ? (%s)\n", __func__, strerror(errno));
            } else {
                PRINT_ERR("%s() msgrcv error : %s\n", __func__, strerror(errno));
            }

            return -1;
        }

        // delay 1ms
        usleep(1000);
    }

    if (server->clt_amt >= MESSAGE_QUEUE_MAX_CLIENT) {
        PRINT_ERR("%s() WARNING Client Register Request BUT Server Full.\n", __func__);
        return -1;
    }

    // check exist client
    msgQ_eve_clt *client = server->head.next;
    while (client != &(server->head))
    {
        if (client->qid == pkg.xid) {
            PRINT_DEBUG("%s() qid %d existed.\n", __func__, client->qid);
            break;
        }

        client = client->next;
    }

    if (client == &(server->head)) {
        // is new client

        client = (msgQ_eve_clt *)omalloc("msgQ event client", sizeof(msgQ_eve_clt));
        if (!client) {
            PRINT_ERR("%s() omalloc failed\n", __func__);
            return -1;
        }
        memset(client, 0, sizeof(msgQ_eve_clt));

        // get client qid
        client->qid = pkg.xid;

        if (server->add(server, client)) {
            PRINT_ERR("%s() server add failed\n", __func__);
            ofree(client);
            return -1;
        }
    }

    tm_set_ms(&(client->timer), 0);

    // response server pid
    pkg.xid = getpid();

    // Send Response
    while (1)
    {
        if (!msgsnd(client->qid, &pkg, sizeof(pkg.xid), 0)) { break; }

        if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN) {
            PRINT_ERR("%s() Queue %d is full (EAGAIN) \n", __func__, client->qid);
            return -1;
        } else {
            PRINT_ERR("%s() Queue %d : %s \n", __func__, client->qid, strerror(errno));
            server->del(server, client);
            return -1;
        }
    }

    return 0;
}
int msgQ_srv_destroy(msgQ_srv *server)
{
    if (!server) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (msgctl(server->qid, IPC_RMID, NULL)) {
        PRINT_ERR("%s() msgctl : %s\n", __func__, strerror(errno));
        return -1;
    }

    // RM client queue ??

    ofree(server);

    return 0;
}
int msgQ_srv_init(msgQ_srv *server, char *name)
{
    if (!server || !name) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    srand(time(NULL));

    server->qid = msgQ_get_qid(name);
    if (server->qid < 0) {
        PRINT_ERR("%s() Get Queue Id failed\n", __func__);
        return -1;
    }

    server->clear(server);

    server->pid = 0;
    server->head.prev = &(server->head);
    server->head.next = &(server->head);

    return 0;
}

msgQ_srv *msgQ_srv_new(void)
{
    msgQ_srv *server = (msgQ_srv *)omalloc("msgQ_srv", sizeof(msgQ_srv));
    if (!server) {
        PRINT_ERR("%s() omalloc failed\n", __func__);
        return NULL;
    }
    memset(server, 0, sizeof(msgQ_srv));

    server->init = msgQ_srv_init;
    server->destroy = msgQ_srv_destroy;

    server->send = msgQ_srv_send;
    server->recv = msgQ_srv_recv;

    server->send_event = msgQ_srv_sendevent;
    server->monitor_request = msgQ_srv_monitorrequest;

    server->len = msgQ_srv_length;
    server->clear = msgQ_srv_clear;

    server->add = msgQ_srv_add;
    server->del = msgQ_srv_del;

    return server;
}
