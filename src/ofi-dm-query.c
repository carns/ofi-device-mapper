/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char** argv)
{

    pid_t my_pid;

    my_pid = getpid();

    printf("This process pid: %d\n", (int)my_pid);

    return (0);
}
