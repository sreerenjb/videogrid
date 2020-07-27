/*
 * VideoGridServer:
 *  Based on Dave O'Hallaron & Carnegie Mellon 's http://www.cs.cmu.edu/afs/cs/academic/class/15213-s00/www/class28/tiny.c
 *   Sreerenj Balachandran  <bsreerenj@gmail.com>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define MAXERRS 16

/*
 * error - wrapper for perror used for bad syscalls
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

/*
 * cerror - returns an error message to the client
 */
void cerror(FILE *stream, char *cause, char *errno, 
	    char *shortmsg, char *longmsg) {
  fprintf(stream, "HTTP/1.1 %s %s\n", errno, shortmsg);
  fprintf(stream, "Content-type: text/html\n");
  fprintf(stream, "\n");
  fprintf(stream, "<html><title>Tiny Error</title>");
  fprintf(stream, "<body bgcolor=""ffffff"">\n");
  fprintf(stream, "%s: %s\n", errno, shortmsg);
  fprintf(stream, "<p>%s: %s\n", longmsg, cause);
  fprintf(stream, "<hr><em>The Tiny Web server</em>\n");
}

int main(int argc, char **argv) {

  /* variables for connection management */
  int parentfd;          /* parent socket */
  int childfd;           /* child socket */
  int portno;            /* port to listen on */
  int clientlen;         /* byte size of client's address */
  struct hostent *hostp; /* client host info */
  char *hostaddrp;       /* dotted decimal host addr string */
  int optval;            /* flag value for setsockopt */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */

  /* variables for connection I/O */
  FILE *stream;          /* stream version of childfd */
  char buf[BUFSIZE];     /* message buffer */
  char method[BUFSIZE];  /* request method */
  char uri[BUFSIZE];     /* request uri */
  char version[BUFSIZE]; /* request method */
  char filename[BUFSIZE];/* path derived from uri */
  char cgiargs[BUFSIZE]; /* cgi argument list */
  char *p;               /* temporary pointer */
  struct stat sbuf;      /* file status */
  int pid;               /* process id from fork */
  int wait_status;       /* status from wait */

  /* check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* open socket descriptor */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
    error("ERROR opening socket");

  /* allows us to restart server immediately */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /* bind port to socket */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* get us ready to accept connection requests */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
    error("ERROR on listen");

  /* 
   * main loop: wait for a connection request, parse HTTP,
   * serve requested content, close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /* wait for a connection request */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) 
      error("ERROR on accept");
    
    /* determine who sent the message */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    
    /* open the child socket descriptor as a stream */
    if ((stream = fdopen(childfd, "r+")) == NULL)
      error("ERROR on fdopen");

    /* get the HTTP request line */
    fgets(buf, BUFSIZE, stream);
    printf("%s", buf);
    sscanf(buf, "%s %s %s\n", method, uri, version);

    /* tiny only supports the GET method */
    if (strcasecmp(method, "GET")) {
      cerror(stream, method, "501", "Not Implemented", 
	     "Tiny does not implement this method");
      fclose(stream);
      close(childfd);
      continue;
    }

    /* read (and ignore) the HTTP headers */
    fgets(buf, BUFSIZE, stream);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {
      fgets(buf, BUFSIZE, stream);
      printf("%s", buf);
    }

    {
      p = index(uri, '?');
      if (p) {
	strcpy(cgiargs, p+1);
	*p = '\0';
      }
      else {
	strcpy(cgiargs, "");
      }
      strcpy(filename, ".");
      strcat(filename, uri);
    }

    /* make sure the file exists */
    if (stat(filename, &sbuf) < 0) {
      cerror(stream, filename, "404", "Not found", 
	     "Tiny couldn't find this file");
      fclose(stream);
      close(childfd);
      continue;
    }

    /* serve dynamic content */
    {
      /* make sure file is a regular executable file */
      if (!(S_IFREG & sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode)) {
	cerror(stream, filename, "403", "Forbidden", 
	       "You are not allow to access this item");
	fclose(stream);
	close(childfd);
	continue;
      }

      /* print first part of response header */
      sprintf(buf, "HTTP/1.1 200 OK\n");
      write(childfd, buf, strlen(buf));
      sprintf(buf, "Server: Tiny Web Server\n");
      write(childfd, buf, strlen(buf));
      sprintf(buf, "Access-Control-Allow-Origin: *\n");
      write(childfd, buf, strlen(buf));

      /* create and run the child CGI process so that all child
         output to stdout and stderr goes back to the client via the
         childfd socket descriptor */
      pid = fork();
      if (pid < 0) {
	perror("ERROR in fork");
	exit(1);
      }
      else if (pid > 0) { /* parent process */
	wait(&wait_status);
      }
      else { /* child  process*/
	close(0); /* close stdin */
	dup2(childfd, 1); /* map socket to stdout */
	dup2(childfd, 2); /* map socket to stderr */
	if (execve(filename, NULL, NULL) < 0) {
	  perror("ERROR in execve");
	}
      }
    }

    /* clean up */
    fclose(stream);
    close(childfd);

  }
}