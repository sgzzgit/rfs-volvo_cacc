#include <stdio.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int OpenTCPConnection(char *local_ip, char* remote_ip, unsigned short local_port, unsigned short remote_port);
int CloseTCPConnection(int sockfd);

// Do not call this function after db_list_init! Doing so created a huge
// problem with binding sockets, probably because somehow db_clt_login closes the stdin
// and stdout file descriptors. When I tried to open a socket, it returned a zero (stdin).

int OpenTCPConnection(char *local_ip, char* remote_ip, unsigned short local_port, unsigned short remote_port){

int sockfd = -1;
	int newsockfd;
	int backlog = 1;
	struct sockaddr_in local_addr;
	socklen_t localaddrlen = sizeof(local_addr);
	struct sockaddr_in remote_addr;

	// Open connection to SMS subnetwork controller
	if( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
	        if(errno != EINTR){
	                perror("socket");
	                return -2;
	        }
	}

	/** set up local socket addressing and port */
	memset(&local_addr, 0, sizeof(struct sockaddr_in));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr =  inet_addr(local_ip);//htonl(INADDR_ANY);//
	local_addr.sin_port = htons(local_port);

	if( (bind(sockfd, (struct sockaddr *)&local_addr,
	        localaddrlen) ) < 0) {
	        if(errno != EINTR){
	                perror("bind");
	                printf("sockfd %d\n", sockfd);
	                close(sockfd);
	                return -3;
	        }
	}

	if( (listen(sockfd, backlog )) < 0) {
	        if(errno != EINTR){
	                perror("listen");
	                close(sockfd);
	                return -4;
	        }
	}

	/** set up remote socket addressing and port */
	memset(&remote_addr, 0, sizeof(struct sockaddr_in));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(remote_ip);//htonl(INADDR_ANY);
	remote_addr.sin_port = htons(remote_port);
	localaddrlen = sizeof(remote_addr);

	if( (newsockfd = accept(sockfd, (struct sockaddr *)&remote_addr,
	&localaddrlen) ) < 0 ) {
	        if(errno != EINTR) {
	                perror("accept");
	                close(sockfd);
	                return -5;
	        }
	}
	return newsockfd;
}

int CloseTCPConnection(int sockfd){
	close(sockfd);
	return 0;
}

