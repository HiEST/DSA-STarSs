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


//int write_data(uint64_t* keys_values);
//void write_data(int pos, uint64_t hash, uint64_t value);
int write_data(int pos, uint64_t hash, uint64_t value);
void hello();
void poll_cq(int num);
int connect_ep();
void register_mapping(size_t len);
int make_context();
int post_some_work();
int post_one_read();
void init_env();
