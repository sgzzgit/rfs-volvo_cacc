/**\file
 *	Receive time information, and set the time
 *	Adjusts time based on the estimated skew value, and replies.
 *	UDP unicast sockets are used
 */

#include <sys_os.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <local.h>
#include <sys_rt.h>
#include <udp_utils.h>
#include "clock_set.h"

static int do_logging = FALSE;

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
	int i;
	int num_items;		// returned from sscanf
	int port = CLOCK_SET_PORT;
	int snd_port = CLOCK_MONITOR_PORT;
	int opt;
	int max_updates = 1;

        while ((opt = getopt(argc, argv, "p:vm:")) != -1) {
                switch (opt) {
		case 'p':
                        port = atoi(optarg);
                        break;
		case 's':
                        snd_port = atoi(optarg);
                        break;
		case 'v':
                        do_logging = TRUE;
                        break;
		case 'm':
                        max_updates = atoi(optarg);
                        break;
		default:
			printf("Usage: %s -p <port> -m <max updates> -v \n", argv[0]);
			break;
		}
	}


	if ((sd = udp_allow_all(port)) < 0){
		printf("%s: receive socket create or bind failed\n", argv[0]);
		exit(1);
	}
	printf("%s receiving on socket %d, port %d\n", argv[0], sd, port);

	if ((sdout = udp_unicast()) < 0) exit(1);

	for (;;) {
		int msg_id;
		int msg_type;
		struct timespec here_tm;
		struct timespec start_tm;
		double here_time;
		double new_time;
		double skew_estimate;
		int socklen = sizeof(snd_addr);
		int retval;
		int origin;

		memset(&snd_addr, 0, socklen);
		if ((bytes_received = recvfrom(sd, 
				&rcv_buffer, RBUFSIZE-1, 0, 
				(struct sockaddr *) &snd_addr,
				&socklen))
			      		<= 0) {
			perror("recvfrom failed");
			break;
		}	

		if (clock_gettime(CLOCK_REALTIME,&here_tm) == -1 ) {
			perror("clock gettime");
		}

		origin = ntohl(snd_addr.sin_addr.s_addr);

                if (do_logging) {
                        printf("Received from 0x%8x, port %d\n",
                                        origin, snd_addr.sin_port);
			printf("%s\n", rcv_buffer);
                }



		num_items = sscanf(rcv_buffer,"%d %d %u %lu %lf",
				&msg_id, &msg_type,
				&start_tm.tv_sec, &start_tm.tv_nsec,
				&skew_estimate);

		// ignore message if bad decode
		if (num_items != 5 || msg_type != CLOCK_SET_MSG_TYPE) {
			printf("num_items %d, msg_type %d\n",
				num_items, msg_type);
			continue;
		}


		// set the time, based on skew estimate from remote
		here_time = timespec2double(here_tm);
		new_time = here_time - skew_estimate;
		here_tm.tv_sec = new_time;	// truncates
		here_tm.tv_nsec = (new_time - here_tm.tv_sec) * 1000000000;

		if(clock_settime(CLOCK_REALTIME, &here_tm) == -1) {
				perror("setclock");
		}
#ifdef __QNXNTO__
		// set the time in the BIOS
		system("/sbin/rtc -s hw");
#endif

		// display seconds mod (24*3600) and nanosecond as millisecond
		printf("%d %d.%03ld %d.%03ld %.6f: bytes %d\n", msg_id,
			start_tm.tv_sec % (24*3600), start_tm.tv_nsec/1000000,
			here_tm.tv_sec % (24*3600), here_tm.tv_nsec/1000000,
			skew_estimate, bytes_received);

		sprintf(rcv_buffer, "%d %d %d %d %ld %d %ld",
			msg_id, CLOCK_SKW_MSG_TYPE,
			bytes_received, start_tm.tv_sec,
			start_tm.tv_nsec,
			here_tm.tv_sec, here_tm.tv_nsec);

		if (bytes_received > 0) {
			int min = strlen(rcv_buffer);
			int j;
			format_buffer_to_send(rcv_buffer,min,
					bytes_received);
		}
		snd_addr.sin_port = htons(snd_port);

		// send back to origin of message received
		retval = sendto(sdout, rcv_buffer, 
				strlen(rcv_buffer)+1,0,
				(struct sockaddr *) &snd_addr,
				sizeof(snd_addr));
		
		if (retval != strlen(rcv_buffer)+1) {
			perror("send to return socket");
			printf("sent % d, length %d, msg id %d\n",
				retval, strlen(rcv_buffer)+1, msg_id);
		}
		if(--max_updates < 0)
			return 0;
	}
	return 0;
}
