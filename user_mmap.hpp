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

#include <iostream>
#include <typeinfo>
#include "common.hpp"
using namespace std;

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
int post_one_retrieve(uint64_t hash);
int retrieve_to_check();
void init_env(void* address);

int post_some_work();
