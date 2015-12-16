#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>

#include <omp.h>
#include "interface.hpp"
#include "common.hpp"

void usage();

void InsertKV(uint64_t key, uint64_t value);

void RetrieveKV();

void IterateKV();