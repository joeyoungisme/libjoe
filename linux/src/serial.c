#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <sys/epoll.h>

#include "misc.h"
#include "shm.h"
#include "serial.h"

int serial_send(serial_class *srl, unsigned char *data, int len)
{
    if (!srl || !data) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    int res = write(srl->fd, data, len);
    if (res < 0) {
        PRINT_ERR("%s() write : %s\n", __func__, strerror(errno));
        return -1;
    }

    return res;
}
int serial_recv(serial_class *srl, unsigned char *data, int len)
{
    if (!srl || !data) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    // Wait Response ( 1 sec timeout )
    struct epoll_event ev = {0};
    if (!epoll_wait(srl->epfd, &ev, 1, 1000)) {
        PRINT_ERR("%s() epoll timeout : no response\n", __func__);
        return -1;
    }

    // When serial data not still recv ( 2ms timeout ), means this is one PDU
    int rxlen = 0;
    while (epoll_wait(srl->epfd, &ev, 1, 2))
    {
        int res = read(ev.data.fd, data + rxlen, len - rxlen);
        if (res < 0) {
            PRINT_ERR("%s() read : %s \n", __func__, strerror(errno));
            return -1;
        }

        rxlen += res;

        if (rxlen >= len) { break; }
    }

    return rxlen;
}

int serial_init(serial_class *srl, char *dev)
{
    if (!srl || !dev) {
        PRINT_ERR("%s() invalid args \n", __func__);
        return -1;
    }

    srl->fd = open(dev, O_RDWR);
    if (srl->fd < 0) {
        PRINT_ERR("%s() open : %s\n", __func__, strerror(errno));
        return -1;
    }

    ioctl(srl->fd, TCGETS, &(srl->tios));
    srl->tios.c_cflag = B115200| CS8 | CLOCAL | CREAD; 
    srl->tios.c_lflag |= ~(ICANON | ECHO);
    srl->tios.c_iflag = 0; 
    srl->tios.c_oflag = 0;
    srl->tios.c_cc[VMIN]=0;    
    srl->tios.c_cc[VTIME]=10; 
	tcflush(srl->fd, TCIFLUSH);
	ioctl(srl->fd, TCSETS, &(srl->tios));

    srl->epfd = epoll_create1(0);
    if (srl->epfd < 0) {
        PRINT_ERR("%s() epoll_create1 : %s\n", __func__, strerror(errno));
        return -1;
    }

    struct epoll_event ev = {
        .data.fd = srl->fd,
        .events = EPOLLIN
    };
    if (epoll_ctl(srl->epfd, EPOLL_CTL_ADD, srl->fd, &ev) < 0)
    {
        PRINT_ERR("%s() epoll_ctl : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}

serial_class *serial_new(void)
{
    static serial_class ins = {
        .init = serial_init,
        .send = serial_send,
        .recv = serial_recv,
    };

    return &ins;
}

