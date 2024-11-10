/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sched.h>

#include <rdma/fabric.h>
#include <rdma/fi_errno.h>

#include <hwloc.h>

struct options {
    char prov_name[256];
};

struct nic {
    char iface_name[256];
    unsigned int bus_id;
    unsigned int device_id;
    unsigned int function_id;
};

#if 0
static int print_short_info(struct fi_info* info);
#endif
static int parse_args(int argc, char** argv, struct options* opts);
static int find_nics(struct options* opts, int* num_nics, struct nic** nics);
static void usage(void);
static int find_cores(struct options* opts, int* num_cores, int* current_core);

int main(int argc, char** argv)
{
    pid_t           my_pid;
    struct options  opts;
    struct nic*     nics = NULL;
    int             num_nics;
    int             num_cores;
    int             current_core;
    int ret;
    int i;

    ret = parse_args(argc, argv, &opts);
    if (ret < 0) {
        usage();
        exit(EXIT_FAILURE);
    }

    /* local process info */
    my_pid = getpid();
    printf("PID:\n\t%d\n", (int)my_pid);

    /* get an array of network interfaces with device ids */
    ret = find_nics(&opts, &num_nics, &nics);
    if(ret < 0) {
        fprintf(stderr, "Error: unable to find network cards.\n");
        return(-1);
    }

    printf("Network cards:\n");
    for(i=0; i<num_nics; i++) {
        printf("\t%s %u %u %u\n", nics[i].iface_name, nics[i].bus_id, nics[i].device_id, nics[i].function_id);
    }

    /* get array of cpu ids */
    ret = find_cores(&opts, &num_cores, &current_core);
    if(ret < 0) {
        fprintf(stderr, "Error: unable to find CPUs.\n");
        return(-1);
    }

    if(nics)
        free(nics);

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

#if 0
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
#endif

static int find_nics(struct options* opts, int* num_nics, struct nic** nics) {
    struct fi_info* info;
    struct fi_info* hints;
    struct fi_info* cur;
    int             ret;
    int i;

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
    hints->fabric_attr->prov_name     = strdup(opts->prov_name);
    /* Depending on the provider we may also need to set the protocol we
     * want; some providers advertise more than one.
     */
    if(strcmp(opts->prov_name, "cxi") == 0) {
        hints->ep_attr->protocol = FI_PROTO_CXI;
    }

    /* This call will populate info with a linked list that can be traversed
     * to inspect what's available.
     */
    ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), NULL, NULL,
                     0, hints, &info);
    if (ret != 0) {
        fprintf(stderr, "fi_getinfo: %d (%s)\n", ret, fi_strerror(-ret));
        return (ret);
    }
    // print_short_info(info);

    /* loop through results and see how many report PCI bus information */
    *num_nics = 0;
    for (cur = info; cur; cur = cur->next) {
        if(cur->nic && cur->nic->bus_attr &&
        cur->nic->bus_attr->bus_type == FI_BUS_PCI)
            (*num_nics)++;
    }

    *nics = calloc(*num_nics, sizeof(**nics));
    assert(*nics);

    /* loop through results again and populate array */
    i=0;
    for (cur = info; cur; cur = cur->next) {
        if(cur->nic && cur->nic->bus_attr &&
        cur->nic->bus_attr->bus_type == FI_BUS_PCI) {
            assert(i<*num_nics);
            strcpy((*nics)[i].iface_name, cur->domain_attr->name);
            (*nics)[i].bus_id = cur->nic->bus_attr->attr.pci.bus_id;
            (*nics)[i].device_id = cur->nic->bus_attr->attr.pci.device_id;
            (*nics)[i].function_id = cur->nic->bus_attr->attr.pci.function_id;
            i++;
        }
    }

    fi_freeinfo(info);
    fi_freeinfo(hints);

    return(0);

}

static int find_cores(struct options* opts, int* num_cores, int* current_core)
{
    hwloc_topology_t topology;
    hwloc_cpuset_t last_cpu;
    hwloc_const_bitmap_t cset_all;
    int sched_cpu;
    int ret;
    hwloc_obj_t obj;
    unsigned i;

    /* Allocate and initialize topology object. */
    hwloc_topology_init(&topology);

    /* ... Optionally, put detection configuration here to ignore
       some objects types, define a synthetic topology, etc....

       The default is to detect all the objects of the machine that
       the caller is allowed to access.  See Configure Topology
       Detection. */

    /* Perform the topology detection. */
    hwloc_topology_load(topology);

    /* query all PUs */
    cset_all = hwloc_topology_get_complete_cpuset(topology);
    *num_cores = hwloc_bitmap_weight(cset_all);

    printf("num_cores: %d\n", *num_cores);

    /* query where this process is running */
    last_cpu = hwloc_bitmap_alloc();
    if(!last_cpu) {
        fprintf(stderr, "hwloc_bitmap_alloc() failure.\n");
        return(-1);
    }
    ret = hwloc_get_last_cpu_location(topology, last_cpu, HWLOC_CPUBIND_THREAD);
    if(ret < 0) {
        fprintf(stderr, "hwloc_get_last_cpu_location() failure.\n");
        return(-1);
    }
    /* print the logical number of the PU where that thread runs */
    /* extract the PU OS index from the bitmap */
    i = hwloc_bitmap_first(last_cpu);
    obj = hwloc_get_pu_obj_by_os_index(topology, i);
    printf("thread is now running on PU logical index %u (OS/physical index %u)\n",
	 obj->logical_index, i);


    /* sanity check vs sched_getcpu */
    sched_cpu = sched_getcpu();
    printf("sched_cpu: %d\n", sched_cpu);

    hwloc_bitmap_free(last_cpu);
    hwloc_topology_destroy(topology);

    return(0);
}
