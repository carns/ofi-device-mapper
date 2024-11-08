/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <rdma/fabric.h>
#include <rdma/fi_errno.h>

struct options {
    char prov_name[256];
};

static int print_short_info(struct fi_info* info);
static int parse_args(int argc, char** argv, struct options* opts);
static void usage(void);

int main(int argc, char** argv)
{
    pid_t           my_pid;
    struct fi_info* info;
    struct fi_info* hints;
    int             ret;
    struct options  opts;

    ret = parse_args(argc, argv, &opts);
    if (ret < 0) {
        usage();
        exit(EXIT_FAILURE);
    }

    /* local process info */
    my_pid = getpid();
    printf("This process pid: %d\n", (int)my_pid);

    /* ofi info */
    hints = fi_allocinfo();
    assert(hints);
    /* These are required as input if we want to filter the results; they
     * indicate functionality that the caller is prepared to provide.  This
     * is just a query, so we want wildcard options except that we must disable
     * deprecated memory registration modes.
     */
    hints->mode = ~0;
    hints->domain_attr->mode = ~0;
    hints->domain_attr->mr_mode = ~3;
    /* Indicate that we only want results matching a particular provider
     * (e.g., cxi or verbs)
     */
    hints->fabric_attr->prov_name     = strdup(opts.prov_name);

    /* This call will populate info with a linked list that can be traversed
     * to inspect what's available.
     */
    ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), NULL, NULL,
                     0, hints, &info);
    if (ret != 0) {
        fprintf(stderr, "fi_getinfo: %d (%s)\n", ret, fi_strerror(-ret));
        return (ret);
    }
    print_short_info(info);
    fi_freeinfo(info);
    fi_freeinfo(hints);

    return (0);
}

static void usage(void)
{
    fprintf(stderr, "Usage: ofi-dm-query -p <provider_name>\n");
    return;
}

static int parse_args(int argc, char** argv, struct options* opts)
{
    int opt;
    int ret;

    memset(opts, 0, sizeof(*opts));

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            ret = sscanf(optarg, "%s", opts->prov_name);
            if (ret != 1) return (-1);
            break;
        default:
            return (-1);
        }
    }

    if (strlen(opts->prov_name) == 0) return (-1);

    return(0);
}


/* derived from info.c in libfabric (BSD license) */
static int print_short_info(struct fi_info* info)
{
    struct fi_info* cur;

    for (cur = info; cur; cur = cur->next) {
        if (cur->nic && cur->nic->bus_attr) printf("bus_attr set!\n");
        printf("provider: %s\n", cur->fabric_attr->prov_name);
        printf("    fabric: %s\n", cur->fabric_attr->name),
            printf("    domain: %s\n", cur->domain_attr->name),
            printf("    version: %d.%d\n",
                   FI_MAJOR(cur->fabric_attr->prov_version),
                   FI_MINOR(cur->fabric_attr->prov_version));
        printf("    type: %s\n",
               fi_tostr(&cur->ep_attr->type, FI_TYPE_EP_TYPE));
        printf("    protocol: %s\n",
               fi_tostr(&cur->ep_attr->protocol, FI_TYPE_PROTOCOL));
    }
    return EXIT_SUCCESS;
}
