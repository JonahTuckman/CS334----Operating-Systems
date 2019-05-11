#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include "request.c"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>]
//

typdef struct __buff_t {
  conn_t *buf;
  int size;
  int *priority;

  pthread_mutex_lock mutex;
  pthread_cond_t full;
  pthread_cond_t sleep;
} buff_t


int main(int argc, char *argv[]) {

    int c;
    char *root_dir = default_root;
    char *scheduler = FIFO;
    int port = 12000;
    int threads = 1;
    int buffers = 1;


    pthread_t MASTER;

    // threadInit(argv[6], argv[8]);
    //requestInit(argv[8]);


    while ((c = getopt(argc, argv, "d:p:t:b:s")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
  case 't':
      threads = atoi(optarg);
      break;
  case 'b':
      buffers = atoi(optarg);
      break;
  case 's':
      scheduler = optarg;
      break;


	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]\n");
	    exit(1);
	}
    // run out of this directory
    chdir_or_die(root_dir);

    // Compare / Assign input scheduler algorithm
    sched_t schedAlgo;
    if(strcmp(scheduler, "FIFO") == 0){
      schedAlgo = FIFO;
    } else if (strcmp(scheduler, "SFF") == 0) {
      schedAlgo = SFF;
    } else {
      fprintf(stderr, "Scheduling Algorithm Not Supported");
      exit(1);
    }

    // INITIALIZATION ERROR CHECKING
    // Buffer initialize
    buff_t buff;
    if(buf_init(&buff, buffers)){
      fprintf(stderr, "Buffer Failure\n" );
      exit(1);
    }

    struct worker_args_t args {
      buff = &buff,
      root = root_dir
    };

    if(create_worker_threads(threads, &args)) {
      fprintf(stderr, "Thread Failure\n", );
      exit(1);
    }

    // now, get to work... blocking until connection comes
    int listen_fd = open_listen_fd_or_die(port);

    connection_t cond;
    if(connection_buffer_init(&cond, buffers)) {
      fprintf(stderr, "Connection Failure\n", );
      exit(1);
    }
    cond.valid = 1;

    sockaddr_t socketaddress;
    socklen_t socketlength;

    while (1) {
        socketlength = sizeof(socketaddress);
	      cond.fd = accept_or_die(listen_fd, &socketaddress, &socketlength);

        if(view_request(&cond)) {
          close_or_die(cond.fd);
          continue;
        }

        pthread_mutex_lock(&buff.lock);
        while(buff.priority[buff.size - 1] != -1) { // if priority is not -1 sleep
          pthread_cond_wait(&buff.full, &buff.lock);
        }

        // Use the scheduler set in command line to schedule the buffer wake up
        schedAlgo(&buff, &cond);

        pthread_cond_signal(&buff.sleep, &buff.lock);

        pthread_mutex_unlock(&buff.lock);
    }
    return 0;
}

// Helpers

typedef struct __connection_t {
  int valid; // check for allocation
  int fd; // file descriptor
  int dynamic; // dynamic or is_static
  size_t filesize; // size of the connection requests
  mode_t mode; // mode of request

  char *method; // HTTP
  char *uri; // URI
  char *version;
  char *filename; // PATH
  char *filetype; // Dyn of Stat
  char *cgiargs; // args for cgi
} connection_t;


int connection_init(conn_t *connection) {
    connection->valid = 0; // All begin unvalidated

    connection-> fd = -1;
    connection->dynamic = -1;
    connection->fileSize = -1;
    connection->filemode = -1;

    // initialize space and add end command
    connection->method = (char *)malloc(MAXBUF);
    connection->method[MAXBUF - 1] = '\0';
    if(connection->method == NULL) return 1;

    connection->uri = (char *)malloc(MAXBUF);
    connection->uri[MAXBUF - 1] = '\0';
    if(connection->uri == NULL) return 1;


    connection->filename = (char *)malloc(MAXBUF);
    connection->filename[MAXBUF - 1] = '\0';
    if(connection->filename == NULL) return 1;


    connection->filetype = (char *)malloc(MAXBUF);
    connection->filetype[MAXBUF - 1] = '\0';
    if(connection->filetype == NULL) return 1;


    connection->version = (char *)malloc(MAXBUF);
    connection->version[MAXBUF - 1] = '\0';
    if(connection->version == NULL) return 1;


    connection->cgiargs = (char *)malloc(MAXBUF);
    connection->cgiargs[MAXBUF - 1] = '\0';
    if(connection->cgiargs == NULL) return 1;

    return 0; // Have intialized all with success
}

int connection_buffer_init(buff_t *conbuf, int size){ // return 0 on success
  conbuf->buf = (buff_t *)malloc(sizeof(buff_t) * size); // Size of buff_t * size of buffer
  if(conbuf->buf == NULL) return 1; // failure

  int error = 0;
  for(int i = 0; i< size; i++) {
    if((error = connection_init(&conbuf-> buf[i]))){
      return error; // Will stay 0 if success, 1 if failure
    }
  }

  conbuf->priority = (int *)malloc(sizeof(int) * size);
  if(conbuf->priority == NULL) return 1; // Failure
  for(int i = 0; i < size; i++){
    conbuf->priority[i] = -1;
  }

  conbuf-> size = size;


  pthread_mutex_init(&conbuf->mutex, NULL);
  pthread_cond_init(&condbuf->sleep, NULL);
  pthread_cond_init(&condbuf->full, NULL);

  return 0; // Gotten through without error... success
}


void *worker_thread(void *args) {
  worker_args_t *worker_args = (worker_args_t*)args;
  buff_t *conbuf = worker_args->buff;
  chdir_or_die(worker_args->root_dir);

  connection_t request;
  connection_init(&request);

  while(1) {
    pthread_mutex_lock(&conbuf->lock);
    while(conbuf->priority[0] == -1) {
      pthread_cond_wait(&conbuf->sleep, &conbuf->lock);
    }

    // When woken from the conditional wait and condition still holds
    copy(&request, &conbuf->buff[conbuf->priority[0]]); // copy requests over
    conbuf->buff[conbuf->priority[0]].valid = 0; // set valid to 0
    conbuf->priority[0] = -1; // set priority

    // Shift priorities
    for (int i = 1; i < conbuf->size; i++){
      conbuf->priority[i - 1] = conbuf->priority[i];
    }

    pthread_cond_signal(&conbuf->full, &conbuf->lock);
    pthread_mutex_unlock(&conbuf->lock);

    request_handle(&request);
    close_or_die(request.fd);
  }
}

int create_worker_threads(int numThreads, worker_args_t *args){
    pthread_t thread;

    for (int i = 0; i < numThreads; i++){
      if(pthread_create(&thread, NULL, worker_thread,args)) {
        return 1;
      }
    }
    return 0;
}

void copy(connection_t *destination, connection_t *source){
  // map destination to source
  // integer copy
  destination->valid = source->valid;
  destination->fd = source->fd;
  destination->dynamic= source->dynamic;
  destination->filesize = source->filesize;
  destination->filemode = source->filemode;

  // string copy
  strncpy(destination->uri, source->uri, MAXBUF);
  strncpy(destination->version, source->version, MAXBUF);
  strncpy(destination->method, source->method, MAXBUF);
  strncpy(destination->filename, source->filename, MAXBUF);
  strncpy(destination->filetype, source->filetype, MAXBUF);
  strncpy(destination->cgiargs, source->cgiargs, MAXBUF);

  return; // void
}


int view_request(connection_t *connection){ // view stat of request (file size)
  char buff[MAXBUF];

  readline_or_die(connection->fd, buff, MAXBUF);
  sscanf(buff, "%s, %s, %s", connection->method, connection->uri, connection->version);

  request_parse_uri(connection);

  struct stat sbuff;
  if(stat(connection->filename, &sbuff)) {
    request_error(connection->fd, connection->filename, "404", "File Not Found",
    "Server Error" );
    return 1;
  }
  connection->filesize = sbuff.st_size; // stat size
  connection->filemode = sbuff.st_mode; // state mode

  return 0;
}


int insert_connection(buff_t *cbuf, connection_t *connection){
  int index = 0;
  while(cbuf->buff[index].valid){
    index++; // find a free index
  }
  copy(&cbuf->buff[index], connection);

  return index;
}

int FIFO(buff_t *cbuff, connection_t *connection){

      int index = insert_connection(cbuff, connection);

      int new_index = 0; // used for new priority index
      while(cbuff->priority[new_index] !- -1) {
        new_index++;// find prioirity with index not equal to -1
      }

      cbuff->priority[new_index] = index;
}

int SFF(budd_t *cbuff, connection_t *connection){

      int index = insert_connection(cbuff, connection);

      int new_index = cbuff->size - 1; // based on size of buffer

      assert(cbuff->priority[new_index] == -1); // index at last index must be -1

      // Sort files by size until find the index needed
      while(cbuff->buff[cbuff->priority[new_index]].filesize > connection->filesize){
        cbuff->priority[new_index + 1] = cbuff->priority[new_index];
        new_index--;
      }

      cbuff->priority[new_index + 1] = index;
}
