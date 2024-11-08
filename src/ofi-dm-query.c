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

static int print_short_info(struct fi_info* info);

int main(int argc, char** argv)
{
    pid_t           my_pid;
    struct fi_info* info;
    struct fi_info* hints;
    int             ret;

    /* local process info */
    my_pid = getpid();
    printf("This process pid: %d\n", (int)my_pid);

    /* ofi info */
    hints = fi_allocinfo();
    assert(hints);
    hints->mode                = FI_ASYNC_IOV;
    hints->ep_attr->type       = FI_EP_RDM;
    hints->caps                = FI_MSG | FI_TAGGED | FI_RMA | FI_DIRECTED_RECV;
    hints->tx_attr->msg_order  = FI_ORDER_NONE;
    hints->tx_attr->comp_order = FI_ORDER_NONE;
    hints->tx_attr->op_flags   = FI_INJECT_COMPLETE;
    hints->domain_attr->resource_mgmt = FI_RM_ENABLED;
    hints->fabric_attr->prov_name     = strdup("tcp");

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

/* from info.c in libfabric (BSD license) */
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
