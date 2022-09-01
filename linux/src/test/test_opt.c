#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "opt.h"

int arg1_action(char *args)
{
    printf("%s() hello ! input : %s\n", __func__, args);
    return 0;
}
int arg2_action(char *args)
{
    printf("%s() hello ! input : %s\n", __func__, args);
    return 0;
}
int arg3_action(char *args)
{
    printf("%s() hello ! input : %s\n", __func__, args);
    return 0;
}

int main(int argc, char *argv[])
{
    struct option arg1 = { .name = "arg1", .has_arg = required_argument };
    opt_reg(arg1, arg1_action, "arg1 description ...");
    struct option arg2 = { .name = "arg2", .has_arg = required_argument };
    opt_reg(arg2, arg2_action, "arg2 description ...");
    struct option arg3 = { .name = "arg3", .has_arg = no_argument };
    opt_reg(arg3, arg3_action, "arg3 description ...");

    if (argc < 2) {
        opt_usage(argv[0]);
        opt_free();
        exit(EXIT_FAILURE);
    } else {
        opt_run(argc, argv);
        opt_free();
    }

    exit(EXIT_SUCCESS);
}
