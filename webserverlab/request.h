#ifndef __REQUEST_H__
#define __REQUEST_H__


#define MAXBUF (8192)

void request_handle(int fd);
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void request_read_headers(int fd);
int request_parse_uri(char *uri, char *filename, char *cgiargs);
void request_get_filetype(char *filename, char *filetype);
void request_serve_dynamic(int fd, char *filename, char *cgiargs);
void request_serve_static(int fd, char *filename, int filesize);
#endif // __REQUEST_H__
