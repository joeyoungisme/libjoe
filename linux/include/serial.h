#ifndef MISC_SERIAL_H
#define MISC_SERIAL_H

#include <termios.h>

typedef struct _serial_class {
    int fd;
    int epfd;

	struct termios tios;

    int (*init)(struct _serial_class *, char *);
    int (*send)(struct _serial_class *, unsigned char *, int);
    int (*recv)(struct _serial_class *, unsigned char *, int);

    // no need ring buff this time
    // uint8_t txbuff[SERIAL_BUFF_SIZE];
    // uint8_t rxbuff[SERIAL_BUFF_SIZE];
} serial_class;

serial_class *serial_new(void);

#endif

