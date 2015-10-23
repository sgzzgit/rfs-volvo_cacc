/**\file
 *	Receive time information, and reply to the system that sent it
 *	Return messages are padded to same length as message received,
 *	unless otherwise specified with -n flag
 */

#include <sys_os.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <local.h>
#include <sys_rt.h>
#include <udp_utils.h>
#include "clock_set.h"

#undef TRACE_RECEIVE

// fill in buffer to desired length
void format_buffer_to_send(char *rcv_buffer, int min, int desired_length)
{
	int j;
	for (j = min; j < desired_length-1 && j < RBUFSIZE-1; j++) {
		if (j == min) // insert space after formatted string
			rcv_buffer[j] = ' ';
		else // fill in with digits
			rcv_buffer[j] = j % 10 + '0';
	}
	rcv_buffer[j] = '\0'; //mark end of string
}

int main(int argc, char **argv)
{
	int sd = -1;		// socket for receive
	int sdout = -1;		// socket for send
	struct sockaddr_in snd_addr;
	char rcv_buffer[RBUFSIZE];	
	int bytes_received;	// received from a call to recv 
	int i, min, max;	// temporary variables
	int opt;
	int do_logging = FALSE;
	int num_bytes = 0;	// if 0, send back same number as received
	int tx_activate = 0;	// if 1, TX options activated and
				// channel cannot be changed
	int channel = 178;	// control channel default
	int power = 10;		// dbm power
	int rate = 2;		// (2 == 6Mbps for 10MHz channel) 
	int port = CLOCK_SET_PORT;
	int snd_port = CLOCK_MONITOR_PORT;
#ifdef WAVE
	sWAVE_Tx_Option tx_option_pkt;
#endif


	if ((sd = udp_allow_all(port)) < 0) exit(1);
	if ((sdout = udp_unicast()) < 0) exit(1);

        while ((opt = getopt(argc, argv, "Ac:p:r:s:n:v")) != -1) {
                switch (opt) {
		case 's':
                        snd_port = atoi(optarg);
                        break;
		case 'n':
                       	num_bytes = atoi(optarg); 
                        break;
		case 'v':
                       	do_logging = TRUE; 
                        break;
		default: 
			printf("Usage: %s [-v verbose] -n [number of bytes]\n",
					 argv[0]);
			break;
		}
	}
	for (;;) {
		int msg_id;
		struct timespec here_tm;
		struct timespec start_tm;
		int socklen = sizeof(struct sockaddr_in);
		int num_items;		// returned from scanf
		int msg_type;
		int retval;
		int origin;	// IP address of source of message

		memset(&snd_addr, 0, socklen);	//clear address before receive
		if ((bytes_received = recvfrom(sd, 
				&rcv_buffer, RBUFSIZE-1, 0, 
				(struct sockaddr *) &snd_addr,
				&socklen)) <= 0) {
			if (do_logging) 
				perror("recvfrom error");
			continue;
		}	

		origin = ntohl(snd_addr.sin_addr.s_addr);

		if (clock_gettime(CLOCK_REALTIME,&here_tm) == -1 ) {
			perror("clock gettime");
		}

		if (do_logging) {
			printf("Received from 0x%8x, port %d\n",
					origin, snd_addr.sin_port);
				
			//printf("%s\n", rcv_buffer); // if byte trace needed
		}
		num_items = sscanf(rcv_buffer,"%d %d %u %lu",
				&msg_id, &msg_type, 
				&start_tm.tv_sec, &start_tm.tv_nsec);

		// ignore message if decode has errors
		if (msg_type != CLOCK_SND_MSG_TYPE)
			continue;

		// display seconds mod (24*3600) and nanosecond as millisecond
		if (do_logging) {
			print_ip_address(origin);
			printf("%d %d.%03ld %d.%03ld : length %d rx_option_length\n",
			msg_id,
			start_tm.tv_sec % (24*3600), start_tm.tv_nsec/1000000,
			here_tm.tv_sec % (24*3600), here_tm.tv_nsec/1000000,
			bytes_received);
		}
		sprintf(rcv_buffer, "%d %d %d %d %ld %d %ld ",
			msg_id, CLOCK_MTR_MSG_TYPE, bytes_received, 
			start_tm.tv_sec, start_tm.tv_nsec,
			here_tm.tv_sec, here_tm.tv_nsec);

		min = strlen(rcv_buffer);
		max = num_bytes ? num_bytes : bytes_received;

		format_buffer_to_send(rcv_buffer,min, max);
		snd_addr.sin_port = htons(snd_port);

		// send back to origin of message received
		retval = sendto(sdout, rcv_buffer, 
				strlen(rcv_buffer)+1,0,
				(struct sockaddr *) &snd_addr,
				sizeof(snd_addr));
		
		if (do_logging) {
			perror("send to return socket");
			printf("sent % d, length %d, msg id %d\n",
				retval, strlen(rcv_buffer)+1, msg_id);
		}
	}
	return 0;
}
