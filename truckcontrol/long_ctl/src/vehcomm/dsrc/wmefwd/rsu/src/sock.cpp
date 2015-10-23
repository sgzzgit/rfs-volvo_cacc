#include <wmefwd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>     
#include <unistd.h>    
#include <arpa/inet.h> 
#include <getopt.h>    
#include <unistd.h>    
#include <string.h>
#include <stdio.h>

struct sockaddr_in ipaddr;
int sockfd = -1;

void socket_main () {
	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror ("sock descriptor");
		exit(-1);
	}

	ipaddr.sin_family = AF_INET;
	ipaddr.sin_port = htons (config_data.port);
	ipaddr.sin_addr.s_addr = inet_addr (config_data.ip);
	memset (ipaddr.sin_zero, 0, sizeof (ipaddr.sin_zero));

	if (config_data.direction == OUTGOING) {
		if(bind(sockfd, (struct sockaddr*) &ipaddr, sizeof ipaddr) < 0) {
			perror ("bind socket");
			exit (-1);
		}
	}
}

void getSocketData (uint8_t *buf) {
	int recv_data_len, addr_len = sizeof (ipaddr);
	static unsigned int s2w_counter = 0;
	recv_data_len = recvfrom(sockfd, buf, config_data.length, 0, 
			(struct sockaddr *)&ipaddr, (socklen_t *)&addr_len);
	if (recv_data_len != config_data.length)
		log ("received data length(%d) != expected data length(%d)",
				recv_data_len, config_data.length);

	log ("%d Socket ---> Wave : %d bytes on port %d\n", s2w_counter++, recv_data_len, ntohs(ipaddr.sin_port));
	/* for (int i = 0; i < config_data.length; i++) */
	/* 	log ("%02x ", buf[i]); */
	/* log ("\n"); */
}

void setSocketData (uint8_t *buf) {
	int send_data_len, addr_len = sizeof (ipaddr);
	static unsigned int w2s_counter = 0;
	send_data_len = sendto(sockfd, buf, config_data.length, 0,
			(struct sockaddr *)&ipaddr, addr_len);
	if (send_data_len != config_data.length)
		log ("sent data length(%d) != expected data length(%d)",
				send_data_len, config_data.length);

	log ("%d Wave ---> Socket : %d bytes on port %d\n", w2s_counter++, send_data_len, ntohs(ipaddr.sin_port));
	/* for (int i = 0; i < config_data.length; i++) */
	/* 	log ("%02x ", buf[i]); */
	/* log ("\n"); */
}
