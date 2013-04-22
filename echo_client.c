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


int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;

	if(argc != 2)
		err_quit("usage: ./echo_client <IPaddress>");
	
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5555);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));
	str_cli(stdin, sockfd);
	exit(0);
}	
