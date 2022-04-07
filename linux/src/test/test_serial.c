#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include "misc.h"
#include "mem.h"
#include "serial.h"

char usage[] = 
"usage : test_serial --dev ttyXX --send xxxxxxxx\n"
"\n";

const struct option opt_must[] = {
    {"example", required_argument,   NULL, 0},
    {"example", no_argument,         NULL, 0},
    {NULL, 0, NULL, 0}
};

const struct option opt_ls[] = {
    {"dev",     required_argument,   NULL, 0},
    {"send",    required_argument,   NULL, 0},
    {NULL, 0, NULL, 0}
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PRINT_DEBUG("%s", usage);
        exit(EXIT_FAILURE);
    }

    char dev[128] = {0};
    char data[2048] = {0};

    int opt = 0;
    int opt_idx = 0;
    while ((opt = getopt_long(argc, argv, "", opt_ls, &opt_idx)) != -1)
    {
        if (!strncmp(opt_ls[opt_idx].name, "dev", 3)) {
            snprintf(dev, sizeof(dev), "/dev/%s", optarg);
        } else if (!strncmp(opt_ls[opt_idx].name, "send", 4)) {
            snprintf(data, sizeof(data), "%s", optarg);
        }
    }

    PRINT_DEBUG("Target : %s\n", dev);
    serial_class *srl = serial_new();
    if (!srl) {
        PRINT_ERR("%s() serial new failed\n", __func__);
        exit(EXIT_FAILURE);
    } else if (srl->init(srl, dev)) {
        PRINT_ERR("%s() serial init failed\n", __func__);
        exit(EXIT_FAILURE);
    // } else if (srl->ctl(srl, TIMEOUT, 200)) {
    }

    PRINT_DEBUG("Send : %s\n", data);

    int res = srl->send(srl, data, strlen(data));
    if (res < 0) {
        PRINT_ERR("%s() srl send failed (%d).\n", __func__, res);
        exit(EXIT_FAILURE);
    }

    char buff[2048] = {0};
    res = srl->recv(srl, buff, sizeof(buff));
    if (res < 0) {
        PRINT_ERR("%s() srl recv failed (%d).\n", __func__, res);
        exit(EXIT_FAILURE);
    }
    PRINT_DEBUG("Recv : %s\n", buff);

    PRINT_DEBUG("done.\n");

    exit(EXIT_SUCCESS);
}

