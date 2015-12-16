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
#include <time.h>
#include <iostream>
#include <fstream>

//#include <ctime>

#include <omp.h>
#include "interface.hpp"
#include "main_interface.hpp"
#include "user_mmap.hpp"



void *my_ioaddr_m;

//int num_wr = 100;
size_t len_to_post_m = 40000;

//char buffer_m[40960];
uint64_t buffer_m[NUM_BUCKET];

size_t area_size_m = sizeof buffer_m; 

//**********************************************************************************//



//#################################################################################//
void InsertKV(uint64_t key, uint64_t value)
{

	//printf("****InsertKV Function****\n");

	insert_interface(key, value);	

}

void RetrieveKV(uint64_t key)
{

	//printf("****RetrieveKV Function****\n");
	retrieve_interface(key);	

}

void IterateKV()
{
	printf("***************************************************\n");
	printf("*                  Iteration                      *\n");
	printf("***************************************************\n");
	iterate_interface();

}





//#################################################################################//

int main(int argc, char *argv[])
{
//static int x;
//x = make_context(void);
//hello();



char *filename = argv[1];
char *command = NULL;
int mode = O_RDWR;
int fd, i, j;


//******************************************************************

char hostname[1024];
hostname[1023] = '\0';
gethostname(hostname, 1023);
printf("Hostname: %s\n", hostname);


 ofstream myfile (hostname);
  /*if (myfile.is_open())
  {
    myfile << "This is a line.\n";
    myfile << "This is another line.\n";
    myfile.close();
  }
  else cout << "Unable to open file";
  return 0;
}*/


//*****************************************************************

//const clock_t begin_time;
//const clock_t end_time;

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


	//######################################################################################
	my_ioaddr_m = mmap(NULL, area_size_m, PROT_NONE, MAP_SHARED, fd, 0);
	if (my_ioaddr_m == MAP_FAILED) {
		perror("mmap failed: ");
		return 0;
	} else
		printf("mmap: got %p\n", my_ioaddr_m);

	sleep(1);


	init_env(my_ioaddr_m);
	

	//######################################################################################3
	/*for (int k=0; k<10; k++)
	{

	my_ioaddr_m[k] = mmap(NULL, area_size_m, PROT_NONE, MAP_SHARED, fd, 0);
	if (my_ioaddr_m[k] == MAP_FAILED) {
		perror("mmap failed: ");
		return 0;
	} else
		printf("mmap: got %p\n", my_ioaddr_m[k]);

	sleep(1);


	init_env(my_ioaddr_m[k]);

	}*/

	//######################################################################################3
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
	//const int NUM_KV = 1;
	//const int SIZE = 2;
	//uint64_t key[SIZE];
	//uint64_t value[SIZE];
	



	uint64_t num = 0;
	uint64_t key = 0;
	uint64_t value = 0;
	//uint64_t all_keys[num_keys];
	uint64_t all_keys[NUM_KEYS];

	//int nthreads = omp_get_num_threads();
   	//cout << "Total number of threads: " << nthreads << endl;  

	printf("***************************************************\n");
	printf("*                  Insertion                      *\n");
	printf("***************************************************\n");
	
	const clock_t begin_time_insert = clock();
	
	//#pragma omp for schedule (static)
	//#pragma omp for schedule (static,1)

	for (int i=0; i<NUM_KEYS; i++)
	{	


		//int ithread = omp_get_thread_num();


      //cout << "Thread number: "  << ithread << " --> inserting key " << i <<endl;
      
      
	//num = rand();

	//key = num;
	//value = num;
	// Let's make it simple as first step	
	key = i;
	value = i;
	//value = 30;
	all_keys[i] = key;

	//usage();

	//cout << "insertion number:" << i << endl;
	

	InsertKV(key,value);
	//if (i%5==0)
	poll_cq(1);

	//InsertKV(10,30);
	//poll_cq(1);
	

	//post_some_work();
	//poll_cq(1);
	//RetrieveKV(key);

	//IterateKV();

	}


	/*for (int i=0; i<NUM_KEYS; i++)
	{	
		poll_cq(1);	
	}*/	
	cout << "Elapsed time for insertion:" << float(clock() - begin_time_insert) / CLOCKS_PER_SEC << endl;
	myfile << float(clock() - begin_time_insert) / CLOCKS_PER_SEC << endl;
	//cout << "After insertion" << endl;
	//poll_cq(4);

	//IterateKV();
	printf("***************************************************\n");
	printf("*                   Retrieve                      *\n");
	printf("***************************************************\n");
	//const clock_t begin_time = clock();
	//cout << "Start time for retrieve:" << begin_time / CLOCKS_PER_SEC << endl; 
	//cout << "Start time for retrieve:" << float(begin_time) << endl; 

	const clock_t begin_time_retrieve = clock();
	for (int i=0; i<NUM_KEYS; i++)
	{	
	
		//RetrieveKV(all_keys[i]);
		//poll_cq(1);

	}

	cout << "Elapsed time for retrieve:" << float(clock() - begin_time_retrieve) / CLOCKS_PER_SEC << endl;
	myfile << float(clock() - begin_time_retrieve) / CLOCKS_PER_SEC << endl;
	//end_time = clock();
	//cout << "End time for retrieve:" << end_time / CLOCKS_PER_SEC << endl;
	//cout << "End time for retrieve:" << float(end_time) << endl;

	//cout << "Elapsed time:" << float(clock() - begin_time) / CLOCKS_PER_SEC << endl;
	//collapse_positions();
	//IterateKV();
	//for (i = 0; i < sizeof(buffer_m); i++)
	//	if (buffer_m[i] != 0)
	//		printf("read back %d at %d\n", buffer_m[i], i);



	const clock_t begin_time_iterate = clock();

	//IterateKV();
	cout << "Elapsed time for iteration:" << float(clock() - begin_time_iterate) / CLOCKS_PER_SEC << endl;
	myfile << float(clock() - begin_time_iterate) / CLOCKS_PER_SEC << endl;

return 0;


}
