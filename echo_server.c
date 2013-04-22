#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <signal.h>
#include <unistd.h>           /*  misc. UNIX functions      */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "helper.h"           /*  our own helper functions  */


int main(int argc, char *argv[]) {
	int 		c; // for getopt()
	int 		lflag = 0;
	char    	*lvalue = NULL;
    int			sock;                /*  listening socket          */
    int 		udpsock;	     /*  listening udp socket */
    int			conn;                /*  connection socket         */
    int 		nready;		     /* for select() */
    int			maxfdp1;
    short int port;                     /*  port number               */
    ssize_t n; 		/* for Recvfrom() function */
    socklen_t clilen;
    pid_t childpid;
    fd_set		readfds;		
    struct sockaddr_in servaddr;  /*  socket address structure  */
    struct sockaddr_in cliaddr;	 /* client socket address struct */
    char 	str[INET_ADDRSTRLEN];
    char      buffer[MAX_LINE];      /*  character buffer          */

	while(( c = getopt(argc, argv, "l:")) != -1) {
		switch(c) {
			case 'l':
				lflag = 1;
				lvalue = optarg;
				break;
			case '?':
				if(optopt== 'c') 
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
					fprintf(stderr, "Unknown option `-%c`.\n", optopt);
				else 
					fprintf(stderr, "Unknown option character `\\x%x`.\n", optopt);
					return 1;
			default:
					abort();
		}
	}


	if ( lflag && lvalue) {
		sock = tcp_listen(NULL, lvalue , NULL);

		udpsock = udp_server(NULL, lvalue, NULL);
	}

	Signal(SIGCHLD, sig_chld);

    
	FD_ZERO(&readfds);
	maxfdp1 = max(sock, udpsock) + 1;
    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    for( ; ; ) {
		// initialize the fd set
		FD_SET(sock, &readfds);
		FD_SET(udpsock, &readfds);

		if ( (nready = select(maxfdp1, &readfds, 0,0,0)) < 0) {
            // back to for(), dealing with signal()
			if (errno == EINTR) 
				continue;
			else
				err_sys("Select() failed");
		}

		if(FD_ISSET(sock, &readfds)) {			
			clilen = sizeof(cliaddr);
			/*  Wait for a connection, then accept() it  */
			if ( (conn = accept(sock, (SA *)&cliaddr, &clilen) ) < 0 ) {
	    		perror("ECHOSERV: Error calling accept()\n");
				continue; /* start over the for loop */
			}
		 
			printf("New client connected from port %d and IP %s\n",ntohs(cliaddr.sin_port), inet_ntoa(cliaddr.sin_addr));
		
			if(( childpid = fork()) < 0) 
				err_sys("Fork() failed");
			else if(childpid == 0) {
				close(sock);/* close child's copy of "sock" */
				str_echo(conn); /* process request */
				exit(0); /* exit child process */
			}
			// parent closes connected socket
			close(conn); 
		}
		if (FD_ISSET(udpsock, &readfds)) {
			clilen = sizeof(cliaddr);
			n = recvfrom(udpsock, buffer, MAX_LINE, 0, (SA *)&cliaddr, &clilen);
		//	printf("New client connected from udp port %d and IP %s\n",ntohs(cliaddr.sin_port), inet_ntoa(cliaddr.sin_addr));
			sendto(udpsock, buffer, n, 0, (SA *)&cliaddr, clilen);
		}
	}
	exit(0);
}
