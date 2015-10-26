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
#include "user_mmap.h"


void	*my_ioaddr_m;

int num_wr_m = 1;
size_t len_to_post_m = 40000;

char buffer_m[40960];
size_t area_size_m = sizeof buffer_m;

struct ibv_cq		*my_cq_m = NULL;

int main(int argc, char *argv[])
{
//static int x;
//x = make_context(void);
//hello();


char *filename = argv[1];
	char *command = NULL;
	int mode = O_RDWR;
	int fd, i, j;

	if (argc < 2)
		return 0;

	command = argv[2];

	if (command)
		len_to_post_m = strtod(command, NULL);

	fd = open(filename, mode);
	if (fd < 0) {
		perror("open");
		return 0;
	} 
	printf("%s opened with %d\n", filename, fd);

	my_ioaddr_m = mmap(NULL, area_size_m, PROT_NONE, MAP_SHARED, fd, 0);
	if (my_ioaddr_m == MAP_FAILED) {
		perror("mmap failed: ");
		return 0;
	} else
		printf("mmap: got %p\n", my_ioaddr_m);

	sleep(1);


	init_env(my_ioaddr_m);
	/*if (make_context())
		return 0;
	register_mapping(area_size_m);
	sleep(1);
	if (connect_ep())
		return 0;
	printf("connected...\n");
	sleep(1);
	if (ibv_req_notify_cq(my_cq_m, 0) != 0) {
		perror("ibv_req_notify_cq");
		return 0;
	}*/
	//post_some_work();
	//******************************************//
	const int NUM_KV = 1;
	const int SIZE = 2;
	//uint64_t key[SIZE];
	//uint64_t value[SIZE];
	
	uint64_t keys_values[SIZE];

	
	for (i=0; i<NUM_KV; i++)
	{
	for (j=0; j<SIZE; j++)
	{
		keys_values[j] = 3;
	}

	write_data(&keys_values);
	}
	printf("posted...\n");
	poll_cq(num_wr_m);
	//read_data();
	post_one_read();
	poll_cq(1);
	//for (i = 0; i < sizeof(buffer_m); i++)
	//	if (buffer_m[i] != 0)
	//		printf("read back %d at %d\n", buffer_m[i], i);





return 0;


}
