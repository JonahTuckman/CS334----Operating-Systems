#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>]
//

// Helpers
//
//
int counter =0;






typedef struct node {
    connection_t *info;
    struct node *prev;
} node_t;

typedef struct queue {
    node_t *head;
    node_t *tail;
    int size;
    int limit;
    pthread_mutex_t *lock;
    pthread_cond_t *full;
    pthread_cond_t *sleep;
} queue_t;
/*
typedef struct __buff_t {
  connection_t *buff;
  int size;
  int *priority;
} buff_t;
*/
typedef struct __worker_args_t {
	queue_t *cqueue;
	char *root;
} worker_args_t;

typedef void(*sched_t)(queue_t *queue, connection_t *connection);


int connection_init(connection_t *connection);
queue_t *connection_queue_init(int limit);
void copy(connection_t *destination, connection_t *source);
void *worker_thread(void *args);
int create_worker_threads(int numThreads, worker_args_t *args);
int view_request(connection_t *connection);
bool insert_connection(queue_t *queue, node_t *item);
void FIFO(queue_t *queue, connection_t *connection);
void SFF(queue_t *queue, connection_t *connection);
bool isEmpty(queue_t *queue);
bool isFull(queue_t *queue);
node_t *pop(queue_t *queue);
int minIndex(queue_t *queue, int sortedIndex);
void insertMinToRear(queue_t *queue, int min_index);
void sortQueue(queue_t *queue);

// Defining functions that exist outside of the main

int connection_init(connection_t *connection) {

    printf("\nHERE IN Connection Init\n");
    connection->valid = 0; // All begin unvalidated

    connection->fd = -1;
    connection->dynamic = -1;
    connection->filesize = -1;
    connection->mode = -1;
    printf("\nConnect Init 1\n");
    // initialize space and add end command
    connection->method = (char *)malloc(MAXBUF);
    connection->method[MAXBUF - 1] = '\0';
    if(connection->method == NULL){ return 1;}
printf("\nConnect Init 2\n");
    connection->uri = (char *)malloc(MAXBUF);
    connection->uri[MAXBUF - 1] = '\0';
    if(connection->uri == NULL) {return 1;}

printf("\nConnect Init 3\n");
    connection->filename = (char *)malloc(MAXBUF);
    connection->filename[MAXBUF - 1] = '\0';
    if(connection->filename == NULL){ return 1;}
printf("\nConnect Init 4\n");

    connection->filetype = (char *)malloc(MAXBUF);
    connection->filetype[MAXBUF - 1] = '\0';
    if(connection->filetype == NULL) {return 1;}
printf("\nConnect Init 5\n");

    connection->version = (char *)malloc(MAXBUF);
    connection->version[MAXBUF - 1] = '\0';
    if(connection->version == NULL) {return 1;}

printf("\nConnect Init 6\n");
    connection->cgiargs = (char *)malloc(MAXBUF);
    connection->cgiargs[MAXBUF - 1] = '\0';
    if(connection->cgiargs == NULL) {return 1;}
printf("\nConnect Init 7\n");
    return 0; // Have intialized all with success
}


queue_t *connection_queue_init(int limit){ // return 0 on success

    queue_t *queue = (queue_t *) malloc(sizeof (queue_t));
    if (queue == NULL) {
        return NULL;
    }
    if (limit <= 0) {
        limit = 65535;
    }
    queue->limit = limit;
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;


 // pthread_mutex_t *mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  //pthread_cond_t *sleep_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
 // pthread_cond_t *full_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
  //queue->lock = mtx;
 // queue->sleep = sleep_cond;
 // queue->full= full_cond;

  queue->lock =(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  queue->sleep = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
 queue->full = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));

  pthread_mutex_init(queue->lock, NULL);
  pthread_cond_init(queue->sleep, NULL);
  pthread_cond_init(queue->full, NULL);

   return queue;
}


void copy(connection_t *destination, connection_t *source){
  // map destination to source
  // integer copy
  destination->valid = source->valid;
  destination->fd = source->fd;
  destination->dynamic = source->dynamic;
  destination->filesize = source->filesize;
  destination->mode = source->mode;

  // string copy
  strncpy(destination->uri, source->uri, MAXBUF);
  strncpy(destination->version, source->version, MAXBUF);
  strncpy(destination->method, source->method, MAXBUF);
  strncpy(destination->filename, source->filename, MAXBUF);
  strncpy(destination->filetype, source->filetype, MAXBUF);
  strncpy(destination->cgiargs, source->cgiargs, MAXBUF);

  return; // void
}




void *worker_thread(void *args) {

 // counter++;

  printf("\nWorker Thread %d Created\n", (int)pthread_self());

  worker_args_t *worker_args = (worker_args_t*)args;
  queue_t *queue = worker_args->cqueue;
  chdir_or_die(worker_args->root);

  node_t *request;

  while(1) {

    pthread_mutex_lock(queue->lock);

    while(isEmpty(queue)) {
      printf("\nWoker %d is asleep\n", (int)pthread_self());
      pthread_cond_wait(queue->sleep, queue->lock);
    }

    if(!isFull(queue)){
      printf("\nQueue is Not full\n");
      pthread_cond_signal(queue->full);
    }
    printf("\nWoker %d is working\n", (int)pthread_self());


    request = pop(queue);

    printf("\nPop request %d \n", request->info->fd);

pthread_mutex_unlock(queue->lock);

   // pthread_mutex_unlock(queue->lock);

//printf("\npop Here1\n");
  printf("\nValid %d\n", request->info->valid);
  printf("\nfd %d\n", request->info->fd);
  printf("\ndynamic %d\n", request->info->dynamic);
   printf("\nmethod %s\n", request->info->method);
   printf("\nuri %s\n", request->info->uri);
   printf("\nversion %s\n", request->info->version);
  printf("\nfilename %s\n", request->info->filename);
  printf("\nfilesize %d\n", request->info->filesize);
   printf("\ncgiargs %s\n", request->info->cgiargs);
    request_handle(request->info);
    //printf("\npop Here2\n");
    //send(request->info->fd, )
     //send(request->info->fd, message, strlen(message), 0);

   //sleep(10);

    close_or_die(request->info->fd);
    //pthread_mutex_unlock(queue->lock);
/*
    free(request->info->method);
    free(request->info->uri);
    free(request->info->version);
    free(request->info->filename);
    free(request->info->cgiargs);
    */
    //free(request->info);
   // free(request);
   pthread_mutex_unlock(queue->lock);
    printf("\npop Here3\n");
  }
}

int create_worker_threads(int numThreads, worker_args_t *args){
    pthread_t thread;

    for (int i = 0; i < numThreads; i++){
      if(pthread_create(&thread, NULL, worker_thread,args) != 0) {
        return -1;
      }
    }
    return 0;
}



int view_request(connection_t *connection){ // view stat of request (file size)
  char buff[MAXBUF];
  const char s[2] = " ";
   char *token;

   /* get the first token */


  readline_or_die(connection->fd, buff, MAXBUF);


  //sscanf(buff, "%s, %s, %s", connection->method, connection->uri, connection->version);


 token = strtok(buff, s);

 char* token1 = strtok(NULL, s);

 char* token2 = strtok(NULL, s);


connection->method = token;

connection->uri = token1;

connection->version = token2;



  request_parse_uri(connection);


  //sprintf(connection->filename, "./%s", "index.html");

  struct stat sbuff;
  if(stat(connection->filename, &sbuff)) {
    request_error(connection->fd, connection->filename, "404", "File Not Found",
    "Server Error" );
    return -1;
  }
  connection->filesize = sbuff.st_size; // stat size
  connection->mode = sbuff.st_mode; // state mode

 printf("\nview request version ~>%s\n",connection->version);


request_get_filetype(connection->uri, connection->filetype);

   printf("\nValid %d\n", connection->valid);
  printf("\nfd %d\n", connection->fd);
  printf("\ndynamic %d\n", connection->dynamic);
  printf("\nmethod %s\n", connection->method);
  printf("\nuri %s\n", connection->uri);
  printf("\nversion %s\n", connection->version);
  printf("\nfilename %s\n", connection->filename);
  printf("\nfilesize%d\n", connection->filesize);
  printf("\ncgiargs %s\n", connection->cgiargs);


  //request_parse_uri(connection);

  return 0;
}

bool insert_connection(queue_t *queue, node_t *item){


if ((queue == NULL) || (item == NULL)) {
        return false;
    }
    if (queue->size >= queue->limit) {
        return false;
    }
    /*the queue is empty*/
    item->prev = NULL;
    if (queue->size == 0) {
        queue->head = item;
        queue->tail = item;

    } else {
        /*adding item to the end of the queue*/
        queue->tail->prev = item;
        queue->tail = item;
    }
    printf("\nBefore QUEUE SIZE %d\n", queue->size);
    queue->size++;
    printf("\nafter QUEUE SIZE %d\n", queue->size);
    return true;

}

void FIFO(queue_t *queue, connection_t *connection){
   printf("\nHERE IN FIFO\n");
    node_t *new_node = (node_t *)malloc(sizeof(node_t));

    new_node->info = malloc(sizeof(connection_t));
    printf("\nHERE IN FIFO: Before Connection init \n");
    connection_init(new_node->info);

    printf("\nHERE IN FIFO: Connection init worked\n");
    copy(new_node->info, connection);
   printf("\nHERE IN FIFO: Copy worked\n");



     if(!insert_connection(queue, new_node)){
      printf("\nFailed to insert connection\n");
     }

     free(connection);

  printf("\nValid %d\n", new_node->info->valid);
  printf("\nfd %d\n", new_node->info->fd);
  printf("\ndynamic %d\n", new_node->info->dynamic);
  printf("\nmethod %s\n", new_node->info->method);
  printf("\nuri %s\n", new_node->info->uri);
  printf("\nversion %s\n", new_node->info->version);
  printf("\nfilename %s\n", new_node->info->filename);
  printf("\ncgiargs %s\n", new_node ->info->cgiargs);




}

void SFF(queue_t *queue, connection_t *connection){
printf("\nHERE IN SFF\n");
node_t *new_node = (node_t *)malloc(sizeof(node_t));
new_node->info = malloc(sizeof(connection_t));
connection_init(new_node->info);
copy(new_node->info, connection);
printf("\nSFF QUEUE SIZE %d\n", queue->size);

 insert_connection(queue, new_node);

        if(queue->size>1){

            sortQueue(queue);
            printf("\nQueue sorted\n");
        }


        printf("\nSFF: inserted :%d\n", new_node->info->fd);


    return;

}


int main(int argc, char *argv[]) {

    int c;
    char *root_dir = default_root;
    char *scheduler = NULL;
    int port = 3456;
    int threads;
    int buffers;


    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1){
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

    queue_t *queue = connection_queue_init(buffers);
    if(queue==NULL){
      fprintf(stderr, "Buffer Failure\n" );
      exit(1);
    }

    printf("\nInitialized Queue\n");


    worker_args_t *args = malloc(sizeof(worker_args_t*));
    args->cqueue = queue;
    args->root = root_dir;



    if(create_worker_threads(threads, args) != 0) {
      fprintf(stderr, "Thread Failure\n");
      exit(1);
    }

     printf("\nCreated Threads\n");

    // now, get to work... blocking until connection comes
    int listen_fd = open_listen_fd_or_die(port);

    /*connection_t *conn = malloc(sizeof(connection_t));
    if(connection_init(conn)) {
      fprintf(stderr, "Connection Failure\n");
      exit(1);
    }
    conn->valid = 1;
*/
    sockaddr_t socketaddress;
    socklen_t socketlength;

    connection_t *conn;
    int new_request_fd;
    printf("\nBefore While in main\n");
    while (1) {

      socketlength = sizeof(socketaddress);

      new_request_fd = accept_or_die(listen_fd, &socketaddress, &socketlength);

      conn = (connection_t*)malloc(sizeof(connection_t));
    if(connection_init(conn)) {
      fprintf(stderr, "Connection Failure\n");
      exit(1);
    }
    conn->valid = 1;

	      conn->fd = new_request_fd;

        printf("\nRequest from client:%d\n", conn->fd);

        if(view_request(conn)) {
          printf("\nhere1\n");
          close_or_die(conn->fd);
          continue;
        }

  printf("\nValid %d\n", conn->valid);
  printf("\nfd %d\n", conn->fd);
  printf("\ndynamic %d\n", conn->dynamic);
  printf("\nmethod %s\n", conn->method);
  printf("\nuri %s\n", conn->uri);
  printf("\nversion %s\n", conn->version);
  printf("\nfilename %s\n", conn->filename);
  printf("\nfilesize%d\n", conn->filesize);
  printf("\ncgiargs %s\n", conn->cgiargs);



        pthread_mutex_lock(queue->lock);
        // printf("\nhere2\n");
        while(isFull(queue)) { // if priority is not -1 sleep
           printf("\nMAIN IS ASLEEP\n");
           printf("\nQUEUE is FULL\n");
          pthread_cond_wait(queue->full, queue->lock);


          printf("\nMain is awake now\n");
        }
        // printf("\nMain is awake now\n");
        // Use the scheduler set in command line to schedule the buffer wake up
        //sched_t schedAlgo;
        schedAlgo(queue, conn);
        //FIFO(queue, conn);


        pthread_cond_signal(queue->sleep);

        pthread_mutex_unlock(queue->lock);
        printf("\nhere5\n");
    }
    printf("\nhere6\n");
    return 0;
}

bool isEmpty(queue_t *queue) {
    if (queue == NULL) {
        return false;
    }
    if (queue->size == 0) {
        return true;
    }
    return false;

}

bool isFull(queue_t *queue) {
    if (queue == NULL) {
        return false;
    }
    if (queue->size == queue->limit){
        return true;
    }
    return false;

}

node_t *pop(queue_t *queue) {
    node_t *item;
    if (isEmpty(queue)){
        return NULL;
    }
    item = queue->head;
    queue->head = (queue->head)->prev;
    queue->size--;
    return item;
}


int minIndex(queue_t *queue, int sortedIndex){
    if (queue==NULL || sortedIndex<0){

        return -1;
    }
    int min_index = -1;
    int min_val = 2147483647;
    int n = queue->size;
    int i;
    for(i=0;i<n;i++){

    node_t *curr_node = pop(queue);

    if((curr_node->info->filesize <= min_val) &&  (i<= sortedIndex)){

        min_index = i;
        min_val = curr_node->info->filesize;

    }
    insert_connection(queue, curr_node);
    }
    return min_index;
}

void insertMinToRear(queue_t *queue, int min_index){

    if (queue==NULL || min_index<0){

        return;
    }
    
    int n;
    int i;
    n=queue->size;
    node_t *last_node;

    for(i=0; i<n; i++){

        node_t *curr_node = pop(queue);

        if(i != min_index){

        insert_connection(queue, curr_node);
        }else{
            last_node = (node_t*) malloc(sizeof (node_t));
            last_node->info = malloc(sizeof(connection_t));
            connection_init(last_node->info);
            copy(last_node->info, curr_node->info);

            last_node->info->valid = 1;

        }
    }

    insert_connection(queue, last_node);

return;
}

void sortQueue(queue_t *queue) {

    if (queue==NULL){

        return;
    }
    int i;
    int n;
    n=queue->size;
    for( i = 1; i <= n; i++)
    {
        int min_index = minIndex(queue, n - i);
        insertMinToRear(queue, min_index);
    }
    return;
}
