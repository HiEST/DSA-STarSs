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
#include <algorithm>
#include <fstream>

#include <omp.h>
#include "interface.hpp"
#include "user_mmap.hpp"


#define KEY_LENGTH 8
//#define NUM_BUCKET 2000

int pos_array[1000];
uint64_t hash_array[1000];

uint64_t MurmurHash64( const void * key, int len, unsigned int seed )
{
	 const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);

    while (data != end) {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch(len & 7) {
        case 7: h ^= uint64_t(data2[6]) << 48;
        case 6: h ^= uint64_t(data2[5]) << 40;
        case 5: h ^= uint64_t(data2[4]) << 32;
        case 4: h ^= uint64_t(data2[3]) << 24;
        case 3: h ^= uint64_t(data2[2]) << 16;
        case 2: h ^= uint64_t(data2[1]) << 8;
        case 1: h ^= uint64_t(data2[0]);
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
	int pos;
    int i = value-1;
    hash = key;
	//printf("****Insert_interface****\n");

	//printf("KEY: %d\n", key);
	//printf("KEY: %" PRIu64 "\n", key);


	//hash = MurmurHash64( &key, KEY_LENGTH, 0);
    //cout << "HASH :" << hash << endl;
	pos = hash % NUM_BUCKET;

    //cout << pos << " --> hash associated first: " << hash << endl;    
	//printf("POSITION: %d\n", pos);

    // Make sure that the new hash will not erase the value associated to another hash inserted always in odd positions
    //cout << "The position at the begining: " << pos << endl;

    /*if (pos % 2 != 0) // position is an odd number/impair
    {
        pos = pos+1;
        //write_data(pos-1, hash, value);       
    }
    else
    {
     
        write_data(pos, hash, value);   
    }*/
    
    //cout << "The position is: " << pos << endl;
    write_data(pos, hash, value);   
	//pos_array[i] = pos; 
    //hash_array[i] = hash; 
	//cout << pos_array[i] << " --> hash associated second: " << hash_array[i] << endl;

	//printf("****after call****\n");
	//poll_cq(1);


}

void collapse_positions()
{
   int mycount;
   int total_collapse = 0;
   ofstream myfile;
    myfile.open ("position.txt");
    for(int i=0; i<1000; i++)
    {
        mycount = count(pos_array, pos_array+1000, pos_array[i]);
        if(mycount != 1)
        {
            total_collapse++;
            cout << pos_array[i] <<" appears " << mycount << " times --> hash associated: " << hash_array[i] << endl;    
           
        }
         myfile << pos_array[i] << endl;
    }
    cout << "Number of collapse : " << total_collapse << endl;




    
    
    myfile.close();



    /*int var;
    int total_positions = 0;
    int count_array[1000];
    for(int i=0; i<1000; i++)
    {
        var = pos_array[i];
        for (int j=0; j<1000; j++)
        {
            if (var == pos_array[j])
        }
    }*/
}

void retrieve_interface(uint64_t key)
{

    uint64_t hash;
    int pos;

    hash = key;
    //printf("****retrieve_interface****\n");

    //printf("KEY: %d\n", key);
    //printf("KEY: %" PRIu64 "\n", key);

    //hash = MurmurHash64( &key, KEY_LENGTH, 0);
    //cout << "HASH :" << hash << endl;

    
    post_one_retrieve(hash);
    //poll_cq(1);


}

/*void retrieve_interface(const void* key)
{



}*/

void iterate_interface()
{
	post_one_read();
	poll_cq(1);

}
