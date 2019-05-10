#include <stdio.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>]
//
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 12000;

    pthread_t MASTER;

     threadInit(argv[6], argv[8]);
     requestInit(argv[8]);


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
	pthread_create(&MASTER, NULL, createPool, poolSize); // Create MASTER Thread
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

//volatile int numWorkers = 0;
//volatile int numThreads = 0;
volatile int count = 0; // incremented when adding a thread to buffer to show order of arival

struct thread {
  pthread_t poolThread;

  int poolSize;
  int numWorkers;

  int numThreads;

	cond_t requests;
	cond_t threads;
	mutex_t lock;
}

struct clients {
  int id;
  int counter;
  int fileSize;
}

struct clients buffer[bufferSize];

void threadInit(int poolSize, int bufferSize){
	pthread_cond_init(&thread-> threads);
	pthread_cond_init(&thread -> requests);
	pthread_mutex_init(&thread->lock);
	//thread->bufferSize = bufferSize; // 8th argument is number of buffers
	thread->poolSize = poolSize;  // 6th argument is size of pool thread
  thread->numWorkers = 0;
  thread->numThreads = 0;
}

void requestInit(int bufferSize){

    pthread_mutex_lock(&thread->lock);

    for(int i = 0; i < bufferSize; i++){
      count++;
      buffer[i].id = i;
      buffer[i].counter = count;
      buffer[i].fileSize = stat(FILENAME!!!!!!!!!);
      pthread_create(&buffer[i], NULL, sleepRequest);
    }

    pthread_mutex_unlock(&thread->lock);
}

void sleepRequest(){
    pthread_mutex_lock(&thread->lock);
    while(thread->numWorkers == thread->poolSize){
      pthread_cond_wait(&thread->requests, &thread->lock);
    }
    thread->numWorkers++;
    pthread_cond_signal(&thread->threads, &thread->lock);

    pthread_mutex_unlock(&thread->lock);
}


void createPool(int poolSize){
	// make threads wake and sleep PoolSize number of times
	// Add threads to a pool buffer?
  pthread_mutex_lock(&thread->lock);

  for(int i = 0; i < poolSize; i++){
    thread->numWorkers++;
    pthread_create(&thread->poolThread, NULL, sleepPool);
	}

  pthread_mutex_unlock(&thread->lock);
}

void sleepPool(){
    pthread_mutex_lock(&thread->lock);

    while(1){
      pthread_cond_wait(&thread->threads, &thread-> lock);
      request_handle(FILENAME!!!!);
      thread->numWorkers--;

      /////// FIFO ////////
      int minIndex = FIFO(buffer);
      pthread_cond_signal(&thread[minIndex]->request, &thread->lock);

      //////// SFF ///////////
      int minFileSize = SFF(buffer);
      pthread_cond_signal(&thread[minFileSize]->request, &thread->lock);
    }

    //request_handle();
    pthead_mutex_unlock(&thread->lock);
}

int FIFO(struct clients buffer[]){
      int length = sizeof(buffer) / sizeof(int);
      int min = 100000;
      temp = 0;
      for(int i = 0; i < length; i++){
          if(buffer[i].count < min){
            min = buffer[i].count;
            temp = i;
          }
      }
      return i;
}

int SFF(struct clients buffer[]){
      int length = sizeof(buffer) / sizeof(int);
      int min = 100000000;
      index = 0;
      for(int i = 0; i < length; i++){
          if(buffer[i].fileSize < min){
            min = buffer[i].fileSize;
            index = i;
          }
      }
      //// SORT FILES IN ORDER ????
      /// Must then remove element at that index fromt the array
      return index;
}
