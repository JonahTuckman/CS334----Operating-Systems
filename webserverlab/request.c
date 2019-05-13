#include "io_helper.h"
#include "request.h"
//#include "wserver.c"

//
// Some of this code stolen from Bryant/O'Halloran
// Hopefully this is not a problem ... :)
//

#define MAXBUF (8192) // maximum value possible

void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXBUF], body[MAXBUF];

    // Create the body of error message first (have to know its length for header)
    sprintf(body, ""
	    "<!doctype html>\r\n"
	    "<head>\r\n"
	    "  <title>OSTEP WebServer Error</title>\r\n"
	    "</head>\r\n"
	    "<body>\r\n"
	    "  <h2>%s: %s</h2>\r\n"
	    "  <p>%s: %s</p>\r\n"
	    "</body>\r\n"
	    "</html>\r\n", errnum, shortmsg, longmsg, cause);

    // Write out the header information for this response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    write_or_die(fd, buf, strlen(buf));

    sprintf(buf, "Content-Type: text/html\r\n");
    write_or_die(fd, buf, strlen(buf));

    sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    write_or_die(fd, buf, strlen(buf));

    // Write out the body last
    write_or_die(fd, body, strlen(body));
}

//
// Reads and discards everything up to an empty text line
//
void request_read_headers(int fd) {
    char buf[MAXBUF];

    readline_or_die(fd, buf, MAXBUF);
    printf("\n%s\n",buf);
    while (strcmp(buf, "\r\n")) {
	readline_or_die(fd, buf, MAXBUF);
    printf("\n%s\n",buf);
    }

    printf("\n%s\n",buf);
    return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
// Rewriting to set dynamic bit rather than return a value
void request_parse_uri(connection_t *connection) {
    char *cgiargs;

    if (strstr(connection->uri, ".cgi")) {
	// static
    connection->dynamic = 1;
    cgiargs = strchr(connection->uri, '?');
    if(cgiargs){ // if above calculation is 1
      strcpy(connection->cgiargs,cgiargs+1); // write args + 1
      *cgiargs = '\0'; // end line
    } else{
      strcpy(connection->cgiargs,"");
    }
  } else {
    connection->dynamic = 0; //dynamic
    strcpy(connection->cgiargs,"");
  }

  // find the file name
  // Change to index.html or spin.cgi  to work 
  sprintf(connection->filename, ".%s", connection->uri);
  
  
   if(connection->uri[strlen(connection->uri) - 1] == '/'){ // pathname of URI
    strcat(connection->filename, "index.html"); // run index file to launch html web browser
  }
  
}

//
// Fills in the filetype given the filename
//
void request_get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else
	strcpy(filetype, "text/plain");
}


////// REWRITE to take in a dynamic connection
void request_serve_dynamic(connection_t *connection) {
    char buf[MAXBUF], *argv[] = { NULL };

    // The server does only a little bit of the header.
    // The CGI script has to finish writing out the header.
    sprintf(buf, ""
	    "HTTP/1.0 200 OK\r\n"
	    "Server: OSTEP WebServer\r\n");

    write_or_die(connection->fd, buf, strlen(buf));

    

    if (fork_or_die() == 0) {                        // child
	setenv_or_die("QUERY_STRING", connection->cgiargs, 1);   // args to cgi go here
	dup2_or_die(connection->fd, STDOUT_FILENO);              // make cgi writes go to socket (not screen)
	extern char **environ;                       // defined by libc
	execve_or_die(connection->filename, argv, environ);
    } else {
	wait_or_die(NULL);
    }
}

///// REWRITE to take in a static connection
void request_serve_static(connection_t *connection) {
    int srcfd;
    char *srcp,  buf[MAXBUF];

    request_get_filetype(connection->filename, connection->filetype);

    printf("\nstatic: filename %s\n", connection->filename);
    srcfd = open_or_die(connection->filename, O_RDONLY, 0);

    // Rather than call read() to read the file into memory,
    // which would require that we allocate a buffer, we memory-map the file
    srcp = mmap_or_die(0, connection->filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close_or_die(srcfd);

    // put together response
    sprintf(buf, ""
	    "HTTP/1.0 200 OK\r\n"
	    "Server: OSTEP WebServer\r\n"
	    "Content-Length: %d\r\n"
	    "Content-Type: %s\r\n\r\n",
	    connection->filesize, connection->filetype);

    write_or_die(connection->fd, buf, strlen(buf));

    //  Writes out to the client socket the memory-mapped file
    write_or_die(connection->fd, srcp, connection->filesize);
    munmap_or_die(srcp, connection->filesize);
}

// handle a request
// REWRITE to take in a connection to handle
void request_handle(connection_t *connection) {
    //fd is condition of  accept_or_die
    //int is_static;
    struct stat sbuf;

    //char buf[MAXBUF];
  //  char filename[MAXBUF], cgiargs[MAXBUF];

     printf("\nhandler here1\n");
     /*
    if (strcasecmp(connection->method, "GET")) {
	request_error(connection->fd, connection->method, "501", "Not Implemented", "server does not implement this method");
	return;
    }
    */
     printf("\nhandler here2\n");
    //request_read_headers(connection->fd);
     printf("\nhandler here3\n");

 
    if (stat(connection->filename, &sbuf) < 0) { // If the check returns 0 it is dynamic
	request_error(connection->fd, connection->filename, "404", "Not found", "server could not find this file");
	return;
    }
    // find which type of file is requested
    // check for errors then call request_serve with type
    
     //printf("\nhandler here4\n");
   /*
    if(connection->dynamic==1){ 

    request_serve_dynamic(connection);

    } else {

        request_serve_static(connection);
    }
      */  
    if(connection->dynamic==1){ 
	if (!(S_ISREG(connection->mode)) || !(S_IRUSR & connection->mode)) {
	    request_error(connection->fd, connection->filename, "403", "Forbidden", "server could not read this file");
	       return;
        }
	
     printf("\nhandler here5\n");
    request_serve_dynamic(connection);
	} else { 

        if(!(S_ISREG(connection->mode)) || !(S_IXUSR & connection->mode)) {
	    request_error(connection->fd, connection->filename, "403", "Forbidden", "server could not run this CGI program");
	    return;
	}
     printf("\nhandler here6\n");
	request_serve_static(connection);
    }
     printf("\nhandler here7\n");

     
}
