#ifndef PG_SOCK_HELP
#define PG_SOCK_HELP


#include <unistd.h>             /*  for ssize_t data type  */
#include <stdio.h>

#define LISTENQ        1024   /*  Backlog for listen()   */
#define ECHO_PORT      5000
#define MAX_LINE 1000

#define max(a,b)	((a) > (b) ? (a):(b))

struct args {
	long arg1;
	long arg2;
};
struct result {
	long sum;
};

#define SA	struct sockaddr
typedef void Sigfunc(int);


/*  Function declarations  */

ssize_t Readline(int fd, void *vptr, size_t maxlen);
ssize_t Writeline(int fc, const void *vptr, size_t maxlen);
void str_echo(int sockd);
void str_cli(FILE *fp, int sockfd);
void sig_chld(int signo);
Sigfunc* Signal(int signo, Sigfunc *func);
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp);
int udp_server(const char *host, const char *serv, socklen_t *addrlenp);
char * Fgets(char *ptr, int n, FILE *stream);
int Socket(int family, int type, int protocol);
ssize_t	writen(int fd, const void *vptr, size_t n);
void Writen(int fd, void *ptr, size_t nbytes);
void Fputs(const char *ptr, FILE *stream);

void err_sys(const char *fmt, ...);

#endif  /*  PG_SOCK_HELP  */
