#include <stdio.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
	pthread_t pool;
	pthread_t requests;
    char *root_dir = default_root;
    int port = 12000;
	int bufferSize = argv[8]; // 8th argument is number of buffers
	int poolSize = argv[6];  // 6th argument is size of pool thread
    cond_t threads; 
	cond_t requests;
	lock_t lock;
	

    while ((c = getopt(argc, argv, "d:p:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}
    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	// Add a lock here
	pthread_create(&pool, NULL, createPool, poolSize); // Create MASTER Thread
	pthread_create(&requests, NULL, handleRequests, bufferSize);
	request_handle(conn_fd);
	close_or_die(conn_fd);
    }
    return 0;
}

//// STEPS ////
// 1. Create a master thread that creates a fixed number (specified in the command line) of threads 
// 		and adds them to a pool of worker threads.
// 2. Master thread then accepts requests and places them into a fixed size (specified in the command line)
// 		buffer.
// 3. Producer - Consumer problem with threads and requests
//		Producer = Worker Threads
//		Consumer = Requests




void createPool(int poolSize){
	// make threads wake and sleep PoolSize number of times
	// Add threads to a pool buffer?
	for(int i = 0; i < poolSize; i++){
	
	}
}

void handleRequests(int bufferSize){
	// Call request handle to error check
	// Signal a sleeping thread
	// decrement buffered requests total
}
 
