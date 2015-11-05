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
#include <inttypes.h>

#include <omp.h>
#include "interface.h"
#include "user_mmap.h"


#define KEY_LENGTH 8
#define NUM_BUCKET 8

uint64_t MurmurHash64( const void * key, int len, unsigned int seed )
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t * data = (const uint64_t *)key;
	const uint64_t * end = data + (len/8);

	while(data != end)
	{
		uint64_t k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h ^= k;
		h *= m; 
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(len & 7)
	{
	case 7: h ^= (uint64_t)data2[6] << 48;
	case 6: h ^= (uint64_t)data2[5] << 40;
	case 5: h ^= (uint64_t)data2[4] << 32;
	case 4: h ^= (uint64_t)data2[3] << 24;
	case 3: h ^= (uint64_t)data2[2] << 16;
	case 2: h ^= (uint64_t)data2[1] << 8;
	case 1: h ^= (uint64_t)data2[0];
	        h *= m;
	};
 
	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
} 

void insert_interface(uint64_t key, uint64_t value)
{

	uint64_t hash;
	uint64_t value2;
	int pos;

	hash = 20;
	value2 = 20;
	pos = 2;

	printf("****Insert_interface****\n");

	/*printf("KEY: %d\n", key);
	printf("KEY: %" PRIu64 "\n", key);


	hash = MurmurHash64( &key, KEY_LENGTH, 0);
	printf("HASH: %d\n", hash); 
	pos = hash % NUM_BUCKET;
	printf("POSITION: %d\n", pos);*/


	printf("****write data call from interface****\n");
	write_data(pos, hash, value2);	

	printf("****after call****\n");
	poll_cq(1);


}


void retrieve_interface(const void* key)
{



}

void iterate_interface()
{
	post_one_read();
	poll_cq(1);

}
