#VPATH = perfsnap/src
#CC=gcc 


CC=mcc
OMPSS_FLAGS = --ompss
EXTRA = -Wno-write-strings

AVAIV_64_FUNC = -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE


TESTCASES=main_interface

#CFLAGS=${ASF_GLOBAL_CFLAGS} -O2 -I../shared -I$(OFED_DIR)/include -I. -I$(TOP)/hs4/driver -I./perfsnap/include -Wall -funroll-loops
CFLAGS= -I../shared -I$(OFED_DIR)/include -I. -I$(TOP)/hs4/driver -I./perfsnap/include -Wall -funroll-loops

SRC=user_mmap.c interface.c main_interface.c 
#OBJ=$(SRC:.c=.o)




all: $(TESTCASES)


#user_mmap: user_mmap.o
#	$(CC) $(CFLAGS) -o $@ $^ -L$(OFED_DIR)/lib -libverbs -lrdmacm


$(TESTCASES): main_interface.o interface.o user_mmap.o
	$(CC) $(CFLAGS) $(OMPSS_FLAGS) $(AVAIV_64_FUNC) $(EXTRA) -o $(TESTCASES) main_interface.o interface.o user_mmap.o -L$(OFED_DIR)/lib -libverbs -lrdmacm
	#$(CC) $(CFLAGS) -o $(TESTCASES) -L$(OFED_DIR)/lib -libverbs -lrdmacm

main_interface.o: main_interface.c main_interface.h interface.h user_mmap.h
	$(CC) $(CFLAGS) -c main_interface.c


interface.o: interface.c interface.h user_mmap.h
	$(CC) $(CFLAGS) -c interface.c


user_mmap.o: user_mmap.c user_mmap.h
	$(CC) $(CFLAGS) -c user_mmap.c


clean:
	rm -f user_mmap.o interface.o main_interface.o $(TESTCASES)

# include dependency files
#-include $(CSRCS:.c=.d)
