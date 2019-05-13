#ifndef __REQUEST_H__
#define __REQUEST_H__

#define MAXBUF (8192)  // maximum value possible

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct __connection_t {
  int valid; // check for allocation
  int fd; // file descriptor
  int dynamic; // dynamic or is_static
  int filesize; // size of the connection requests
  mode_t mode; // mode of request

  char *method; // HTTP
  char *uri; // URI
  char *version;
  char *filename; // PATH
  char *filetype; // Dyn of Stat
  char *cgiargs; // args for cgi
} connection_t;


void request_handle(connection_t *connection);
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void request_read_headers(int fd);
void request_parse_uri(connection_t *connection);
void request_get_filetype(char *filename, char *filetype);
void request_serve_dynamic(connection_t *connection);
void request_serve_static(connection_t *connection);
#endif // __REQUEST_H__
