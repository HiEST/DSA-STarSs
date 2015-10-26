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

#include "user_mmap.h"

struct ibv_context	*my_dsa_context;
struct ibv_pd		*my_pd;
struct ibv_mr		*my_io_mr, *my_host_mr;
struct rdma_cm_id	*my_id;
struct rdma_event_channel *my_ec;
struct addrinfo 	*my_addr;
struct rdma_cm_event	*my_event;
struct ibv_qp_init_attr	my_qp_attrs;
struct rdma_conn_param	my_cm_params;
struct ibv_cq		*my_cq = NULL;
struct ibv_comp_channel	*my_wc_channel = NULL;

void	*my_ioaddr;

int num_wr = 1;
size_t len_to_post = 40000;

char buffer[40960];
size_t area_size = sizeof buffer; 

const int NUM_KV = 100;

void poll_cq(int num)
{
	struct ibv_cq *cq;
	struct ibv_wc wc;
	void *ctx;
	int polled = 0;
	int rv;

wait_again:
	printf("poll_cq: wait for event\n");
	rv = ibv_get_cq_event(my_wc_channel, &cq, &ctx);
	if (rv < 0) {
		perror("ibv_get_cq_event");
		return;
	}
	ibv_ack_cq_events(cq, 1);
poll_again:
	rv = ibv_poll_cq(cq, 1, &wc);
	if (rv > 0) {
		printf("work completion for QP[%d], id %lu, status %d\n",
			wc.qp_num, wc.wr_id, wc.status); 
		if (++polled < num)
			goto poll_again;
		printf("work completion polling: done\n");
		ibv_req_notify_cq(cq, 0);
		return;
	}
	if (rv == 0) {
		rv = ibv_req_notify_cq(cq, 0);
		if (rv) {
			perror("ibv_req_notify_cq");
			return;
		}
		goto wait_again;
	}
	if (rv < 0)
		perror("poll cq");
}

int connect_ep()
{
	/* Create completion channel and CQ */
	my_wc_channel = ibv_create_comp_channel(my_dsa_context);
	if (!my_wc_channel) {
		perror("wc");
		return -1;
	}
	my_cq = ibv_create_cq(my_dsa_context, num_wr + 1,
			      NULL, my_wc_channel, 0);
	if (!my_cq) {
		perror("cq");
		return -1;
	};
	
	memset(&my_qp_attrs, 0, sizeof my_qp_attrs);
	memset(&my_cm_params, 0, sizeof my_cm_params);
	my_qp_attrs.send_cq = my_cq;
	my_qp_attrs.recv_cq = my_cq;
	my_qp_attrs.qp_type = IBV_QPT_RC;
	my_qp_attrs.cap.max_send_wr = num_wr;
	my_qp_attrs.cap.max_recv_wr = 10;
	my_qp_attrs.cap.max_send_sge = 2;
	my_qp_attrs.cap.max_recv_sge = 1;

	if (rdma_create_qp(my_id, my_pd, &my_qp_attrs)) {
		perror("qp");
		return -1;
	}
	if (rdma_resolve_route(my_id, 200)) {
		perror("rr");
		return -1;
	}
	if (rdma_get_cm_event(my_ec, &my_event)) {
		perror("get event");
		return -1;
	}
	printf("route resolved: %s,  %d\n", rdma_event_str(my_event->event),
		my_event->status);
	rdma_ack_cm_event(my_event);

	if (rdma_connect(my_id, &my_cm_params)) {
		perror("connect");
		return -1;
	}
	if (rdma_get_cm_event(my_ec, &my_event)) {
		perror("get event");
		return -1;
	}
	printf("connected: %s,  %d\n", rdma_event_str(my_event->event),
		my_event->status);
	rdma_ack_cm_event(my_event);

	
	return 0;
}

void register_mapping(size_t len)
{
	if (!my_dsa_context) {
		printf("no dsa device openend\n");
		return;
	}
	my_io_mr = ibv_reg_mr(my_pd, my_ioaddr, len,
			      IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|
			      IBV_ACCESS_REMOTE_READ);
	if (!my_io_mr) {
		perror("ibv_reg_mr:");
		return;
	}
	printf("MR openend, lkey %x (%d), rkey%x\n", my_io_mr->lkey, 
		my_io_mr->lkey >> 8, my_io_mr->rkey);
}

int make_context(void)
{
	if (getaddrinfo("localhost", "4242", NULL, &my_addr)) {
		perror("addrinfo");
		return -1;
	}
	my_ec = rdma_create_event_channel();
	if (!my_ec) {
		perror("ec");
		return -1;
	}
	if (rdma_create_id(my_ec, &my_id, NULL, RDMA_PS_TCP)) {
		perror("id");
		return -1;
	}
	if (rdma_resolve_addr(my_id, NULL, my_addr->ai_addr, 200)) {
		perror("resolve");
		return -1;
	}
	if (rdma_get_cm_event(my_ec, &my_event)) {
		perror("get event");
		return -1;
	}
	printf("addr resolved: %s,  %d\n", rdma_event_str(my_event->event),
		my_event->status);
	rdma_ack_cm_event(my_event);

	my_dsa_context = my_id->verbs;

	my_pd = ibv_alloc_pd(my_dsa_context);
	if (!my_pd) {
		perror("ibv_alloc_pd:");
		return -1;
	}
	return 0;
}


int post_some_work()
{
	struct ibv_send_wr wr, *bad_wr = NULL;
	struct ibv_sge	sge[4];
	uint64_t remote_addr = (uint64_t)my_ioaddr;
	int bytes = 0, i = 0;
	struct ibv_mr *my_small_mr;
	long int value[1] = { 5 };

	memset(buffer, 2, sizeof(buffer));
	/*
	 * register that local buffer
	 */
	my_host_mr = ibv_reg_mr(my_pd, buffer, sizeof buffer,
				IBV_ACCESS_LOCAL_WRITE);
	if (!my_host_mr) {
		perror("ibv_reg_mr:");
		return -1;
	}
	my_small_mr = ibv_reg_mr(my_pd, &value, sizeof value,
				 IBV_ACCESS_LOCAL_WRITE);
	if (!my_small_mr) {
		perror("ibv_reg_mr:");
		return -1;
	}
	while (i++ < num_wr) {
		int len = len_to_post;
		uint64_t end_of_area = (uint64_t)my_ioaddr + area_size;
		uint64_t remote_end = (uint64_t)remote_addr + (uint64_t)len;

		if (remote_end  > end_of_area)
			remote_addr = (uint64_t)my_ioaddr;

		sge[0].addr = (uint64_t)&value;
		sge[0].length = sizeof value;
		sge[0].lkey = my_small_mr->lkey;

		/* Test intermediate null length SGE */
		sge[2].addr = (uint64_t)&value;
		sge[2].length = 0;
		sge[2].lkey = my_small_mr->lkey;

		sge[3].addr = (uint64_t)&value;
		sge[3].length = sizeof value;
		sge[3].lkey = my_small_mr->lkey;

		len -= sge[0].length + sge[2].length + sge[3].length;
		if (len < 0)
			len = 0;
		sge[1].addr = (uint64_t)buffer;
		sge[1].length = len;
		sge[1].lkey = my_host_mr->lkey;
		
		wr.wr_id = i;
		wr.opcode = IBV_WR_RDMA_WRITE;
		wr.sg_list = sge;
		wr.num_sge = 4;
		wr.send_flags = IBV_SEND_SIGNALED;
		wr.wr.rdma.remote_addr = remote_addr;
		wr.wr.rdma.rkey = my_io_mr->rkey;
		wr.next = NULL;

		bytes += sge[0].length + sge[1].length;

		if (ibv_post_send(my_id->qp, &wr, &bad_wr)) {
			perror("post send");
			return -1;
		}
		printf("%d: called post_send with [%u][%u] bytes, raddr %p\n",
			i, sge[0].length, sge[1].length, (void *)remote_addr);
		remote_addr += len;
	}
	printf("finished post_send WRITE with %d bytes pushed\n",
		bytes);
	return 0;
}

int post_one_read()
{
	struct ibv_send_wr wr, *bad_wr = NULL;
	struct ibv_sge	sge;
	int i;
	memset(buffer, 0, sizeof(buffer));

	sge.addr = (uint64_t)buffer;
	sge.length = len_to_post;
	sge.lkey = my_host_mr->lkey;
	wr.wr_id = 4711;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.send_flags = IBV_SEND_SIGNALED;
	wr.wr.rdma.remote_addr = (uint64_t)my_ioaddr;
	wr.wr.rdma.rkey = my_io_mr->rkey;
	wr.next = NULL;

	if (ibv_post_send(my_id->qp, &wr, &bad_wr)) {
		perror("post send read");
		return -1;
	}

	for ( i = 0; i < sizeof(buffer); i++)
                if (buffer[i] != 0)
                        printf("read back %d at %d\n", buffer[i], i);

	return 0;
}

void init_env(void* my_ioaddr_m)
{
	my_ioaddr = my_ioaddr_m;
	if (make_context())
		exit(1);
		//return 0;
	register_mapping(area_size);
	sleep(1);
	if (connect_ep())
		exit(1);
		//return 0;
	printf("connected...\n");
	sleep(1);
	if (ibv_req_notify_cq(my_cq, 0) != 0) {
		perror("ibv_req_notify_cq");
		exit(1);
		//return 0;
	}
}

int write_data(uint64_t* keys_values)
{
	struct ibv_send_wr wr, *bad_wr = NULL;
	//struct ibv_sge	sge[4];
	struct ibv_sge	sge[1];
	uint64_t remote_addr = (uint64_t)my_ioaddr;
	int bytes = 0, i = 0, k;
	struct ibv_mr *my_small_mr;
	long int value[1] = { 4 };

	printf("***0*** : %d\n", keys_values[0]);
	printf("***1*** : %d\n", keys_values[1]);
	
	memset(buffer, 0, sizeof(buffer));
	for(k=0; k<2; k++)
		buffer[k] = keys_values[k];

	/*
	 * register that local buffer
	 */
	my_host_mr = ibv_reg_mr(my_pd, buffer, sizeof buffer,
				IBV_ACCESS_LOCAL_WRITE);
	if (!my_host_mr) {
		perror("ibv_reg_mr:");
		return -1;
	}

	my_small_mr = ibv_reg_mr(my_pd, &value, sizeof value,
				 IBV_ACCESS_LOCAL_WRITE);
	if (!my_small_mr) {
		perror("ibv_reg_mr:");
		return -1;
	}
	while (i++ < num_wr) {
		int len = len_to_post;
		uint64_t end_of_area = (uint64_t)my_ioaddr + area_size;
		uint64_t remote_end = (uint64_t)remote_addr + (uint64_t)len;

		if (remote_end  > end_of_area)
			remote_addr = (uint64_t)my_ioaddr;

		//sge[0].addr = (uint64_t)&value;
		//sge[0].length = 0;
		//sge[0].lkey = my_small_mr->lkey;

		/* Test intermediate null length SGE */
		//sge[2].addr = (uint64_t)&value;
		//sge[2].length = 0;
		//sge[2].lkey = my_small_mr->lkey;

		//sge[3].addr = (uint64_t)&value;
		//sge[3].length = 0;
		//sge[3].lkey = my_small_mr->lkey;

		//len -= sge[0].length + sge[2].length + sge[3].length;
		if (len < 0)
			len = 0;
		//sge[1].addr = (uint64_t)buffer;
		//sge[1].length = len;
		//sge[1].lkey = my_host_mr->lkey;
	
			
		sge[0].addr = (uint64_t)buffer;
		sge[0].length = len;
		sge[0].lkey = my_host_mr->lkey;
		
		wr.wr_id = i;
		wr.opcode = IBV_WR_RDMA_WRITE;
		wr.sg_list = sge;
		//wr.num_sge = 4;
		wr.num_sge = 1;
		wr.send_flags = IBV_SEND_SIGNALED;
		wr.wr.rdma.remote_addr = remote_addr;
		wr.wr.rdma.rkey = my_io_mr->rkey;
		wr.next = NULL;

		//bytes += sge[0].length + sge[1].length;
		bytes += sge[0].length;

		if (ibv_post_send(my_id->qp, &wr, &bad_wr)) {
			perror("post send");
			return -1;
		}
		printf("%d: called post_send with [%u] bytes, raddr %p\n",
			i, sge[0].length, (void *)remote_addr);
		remote_addr += len;
	}
	printf("finished post_send WRITE with %d bytes pushed\n",
		bytes);
	return 0;

}

int read_data()
{
	struct ibv_send_wr wr, *bad_wr = NULL;
	struct ibv_sge	sge;
	uint64_t my_buffer[2];
	
	memset(my_buffer, 0, sizeof(my_buffer));

	sge.addr = (uint64_t)my_buffer;
	//sge.length = sizeof my_buffer;
	sge.length = 8;
	sge.lkey = my_host_mr->lkey;
	wr.wr_id = 4711;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.send_flags = IBV_SEND_SIGNALED;
	wr.wr.rdma.remote_addr = (uint64_t)my_ioaddr;
	wr.wr.rdma.rkey = my_io_mr->rkey;
	wr.next = NULL;

	if (ibv_post_send(my_id->qp, &wr, &bad_wr)) {
		perror("post send read");
		return -1;
	}

	
	printf("Size of my_buffer %d\n", sizeof my_buffer);	
	int i;
	for ( i = 0; i < sizeof(my_buffer); i++)
                //if (my_buffer[i] != 0)
                        printf("read back %d at %d\n", my_buffer[i], i);

	
	return 0;


}

void hello()
{

	printf("hello world");
}

/* Dummy test for mmapping partition */
/*int main(int argc, char *argv[])
{
	char *filename = argv[1];
	char *command = NULL;
	int mode = O_RDWR;
	int fd, i;

	if (argc < 2)
		return 0;

	command = argv[2];

	if (command)
		len_to_post = strtod(command, NULL);

	fd = open(filename, mode);
	if (fd < 0) {
		perror("open");
		return 0;
	} 
	printf("%s opened with %d\n", filename, fd);

	my_ioaddr = mmap(NULL, area_size, PROT_NONE, MAP_SHARED, fd, 0);
	if (my_ioaddr == MAP_FAILED) {
		perror("mmap failed: ");
		return 0;
	} else
		printf("mmap: got %p\n", my_ioaddr);

	sleep(1);
#if 0
	printf("about to write  to%p\n", myaddr);

	sleep(1);

	*myaddr = 42;

	printf("about to read from%p\n", myaddr);

	sleep(1);

	printf("got %d\n", *myaddr);
#endif
	if (make_context())
		return 0;
	register_mapping(area_size);
	sleep(1);
	if (connect_ep())
		return 0;
	printf("connected...\n");
	sleep(1);
	if (ibv_req_notify_cq(my_cq, 0) != 0) {
		perror("ibv_req_notify_cq");
		return 0;
	}
	post_some_work();
	printf("posted...\n");
	poll_cq(num_wr);
	//post_one_read();
	//post_one_read();
	//post_one_read();
	//poll_cq(1);
	
	printf("Size of buffer %d\n", sizeof(buffer));
	
	for (i = 0; i < sizeof(buffer); i++)
	//for (i = 0; i < 2; i++)
		if (buffer[i] != 6)
			printf("read back %d at %d\n", buffer[i], i);

	if (command && !strncmp(command, "unmap", 3)) {
		printf("about to unmap from%p\n", my_ioaddr);

		sleep(5);

		munmap(my_ioaddr, 42);

		printf("unmapped.....about to close %s\n", filename);
		
		sleep(5);

		close(fd);

	} else if (command && !strncmp(command, "wait", 4)) {
		printf("waiting before close %s\n", filename);

		sleep(600);

		close(fd);

	} else if (command && !strncmp(command, "two", 3)) {
		void *myaddr2;

		printf("do another mmap %s\n", filename);

		myaddr2 = mmap(NULL, 42, PROT_NONE, MAP_SHARED, fd, 0);
		if (myaddr2 == MAP_FAILED)
			perror("another mmap failed: ");
		else
			printf("another mmap: got %p\n", myaddr2);
	} else {
		sleep(5);
		close(fd);
	}
	printf("closed %s\n", filename);

	return 0;
}*/
	
