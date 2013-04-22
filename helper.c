
#include "helper.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

#define MAX_RECV_BUF 256

/*  Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {
	
		if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    	*buffer++ = c;
	    	if ( c == '\n' )
			break;
		}
		else if ( rc == 0 ) {
 	   		if ( n == 1 )
				return 0;
	    	else
				break;
		}
		else {
	    	if ( errno == EINTR )
				continue;
	    	return -1;
		}
    }

    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 ) {
	if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
	    if ( errno == EINTR )
		nwritten = 0;
	    else
		return -1;
	}
	nleft  -= nwritten;
	buffer += nwritten;
    }

    return n;
}

void str_echo(int sockd) {
	ssize_t n;
	struct args args;
	struct result result;
	for( ; ; ) {
		if( (n = Readn(sockd, &args, sizeof(args))) == 0)
			return;
		result.sum = args.arg1 + args.arg2;
		Writen(sockd, &result, sizeof(result));
	}
}

void str_cli(FILE *fp, int sockfd) {
	char sendline[MAX_LINE];
	struct args args;
	struct result result;

	while(Fgets(sendline, MAX_LINE, fp) != NULL) {
		if(sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2) {
			printf("args1 = %ld\t args2 = %ld\n",args.arg1, args.arg2);
			printf("invalid input: %s", sendline);
			continue;
		}
		Writen(sockfd,&args, sizeof(args));
		if(Readn(sockfd, &result,sizeof(result)) == 0)
			err_quit("str_cli: server terminated prematurely");
		printf("%ld\n", result.sum);
	}
}

void sig_chld(int signo) {
	pid_t pid;
	int stat;
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0) 
		printf("Child %d terminated\n",pid);
	return;
}
	

Sigfunc * Signal(int signo, Sigfunc *func) {
	struct sigaction act, oact;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if ( signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		// SunOS 4.x
		act.sa_flags |= SA_INTERRUPT;	
#endif
	}
	else {
#ifdef SA_RESTART
		// SVR$, 4.4BSD
		act.sa_flags |= SA_RESTART;
#endif
	}
	if( sigaction(signo, &act, &oact) < 0) 
		return (SIG_ERR);

	return (oact.sa_handler);
}


int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp) {
	int 		listenfd, n;
	const		int on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE; // for wildcard IP address
	hints.ai_family = AF_UNSPEC; // return two socket address structure
	hints.ai_socktype = SOCK_STREAM;
	
	if(( n = getaddrinfo(host, serv, &hints, &res)) != 0) 
		err_quit("tcp_listen error for %s, %s: %s", host, serv, gai_strerror(n));
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(listenfd < 0) 
			continue;	// error, try next one
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		
		if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;	// success, return 0 if ok
		
		close(listenfd); // bind error, close and try next one
	}while((res=res->ai_next)!= NULL);

	if(res == NULL) 
		// tried all the socket address structures, all failed
		err_sys("tcp_listen error for %s, %s", host, serv);
	
	if( listen(listenfd, LISTENQ) < 0)
		err_sys("ECHOSERV: error calling listen()");
	
	if( addrlenp ) 
		*addrlenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return (listenfd);
}

int udp_server(const char *host, const char *serv, socklen_t *addrlenp) {
	int 		sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE; // for wildcard IP address
	hints.ai_family = AF_UNSPEC; // return two socket address structure
	hints.ai_socktype = SOCK_DGRAM;
	
	if(( n = getaddrinfo(host, serv, &hints, &res)) != 0) 
		err_quit("udp_server error for %s, %s: %s", host, serv, gai_strerror(n));
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(sockfd < 0) 
			continue;	// error, try next one
		
		if(bind(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;	// success, return 0 if ok
		
		close(sockfd); // bind error, close and try next one
	}while((res=res->ai_next)!= NULL);

	if(res == NULL) 
		// tried all the socket address structures, all failed
		err_sys("tcp_listen error for %s, %s", host, serv);
	
	if( addrlenp ) 
		*addrlenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return (sockfd);
}

int Socket(int family, int type, int protocol)
{
	int		n;

	if ( (n = socket(family, type, protocol)) < 0)
		err_sys("socket error");
	return(n);
}

void
Connect(int fd, const struct sockaddr *sa, socklen_t salen) {
	if (connect(fd, sa, salen) < 0)
		err_sys("connect error");
}

void Inet_pton(int family, const char *strptr, void *addrptr) {
	int		n;

	if ( (n = inet_pton(family, strptr, addrptr)) < 0)
		err_sys("inet_pton error for %s", strptr);	/* errno set */
	else if (n == 0)
		err_quit("inet_pton error for %s", strptr);	/* errno not set */
	/* nothing to return */
}

char * Fgets(char *ptr, int n, FILE *stream) {
	char	*rptr;

	/* ferror() returns nonzero if it is set */
	if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		err_sys("fgets error");

	return (rptr);
}

ssize_t	writen(int fd, const void *vptr, size_t n) {
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void Writen(int fd, void *ptr, size_t nbytes) {
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}

void Fputs(const char *ptr, FILE *stream) {
	if (fputs(ptr, stream) == EOF)
		err_sys("fputs error");
}

ssize_t						/* Read "n" bytes from a descriptor. */ 
readn(int fd, void *vptr, size_t n) {
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}
/* end readn */

ssize_t
Readn(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = readn(fd, ptr, nbytes)) < 0)
		err_sys("readn error");
	return(n);
}
