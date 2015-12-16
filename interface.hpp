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
#include "user_mmap.hpp"
#include "common.hpp"


uint64_t MurmurHash64( const void * key, int len, unsigned int seed );
void insert_interface(uint64_t key, uint64_t value);
void retrieve_interface(uint64_t key);
void collapse_positions();
//void retrieve_interface(const void* key);
void iterate_interface();
//void iterate();


