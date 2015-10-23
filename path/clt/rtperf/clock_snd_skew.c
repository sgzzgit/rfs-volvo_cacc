/**
 * Send time to remote system; receive replies from remote system and
 * use to get skew estimate to send to remote system
 */
#include <sys_os.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <local.h>
#include <sys_rt.h>
#include <timestamp.h>
#include <udp_utils.h>
#include "clock_set.h"

extern unsigned int src_ip_for_checksum;
extern double read_socket_get_skew(int sd, struct sockaddr_in *paddr,
		int do_logging);
extern void send_time_to_remote(int sd, struct sockaddr_in *paddr, double skew,
			int message_length);

int main(int argc, char **argv)

{
	int sd;
	int rd_sd;
	struct sockaddr_in rcv_addr;
	unsigned short port = CLOCK_MONITOR_PORT;  
	struct sockaddr_in snd_addr;
	unsigned short snd_port = CLOCK_SET_PORT; 
	char *snd_ip = CLOCK_SET_IP;
	int num_sends = 0;	// continuous send
	int timer_interval = 0;	// send as fast as you can
	int message_length = 0;	// pad after formatted string if non-zero
	struct timespec tm_spec;
	int socket_option = 0;  // unicast socket
	int opt;
	int i;
	double skew_estimate = 0.0;
	fd_set readfds;
	int ndesc;
	int bytes_received;
	struct timeval timeout;
	int do_logging = FALSE;
	timestamp_t current_ts, last_ts;

	while ((opt = getopt(argc, argv, "a:m:n:p:s:t:vz:")) != -1) {
                switch (opt)
                {
                  case 'a':
                        snd_ip = strdup(optarg);
                        break;
                  case 'n':
                        num_sends = atoi(optarg);
                        break;
                  case 'p':
                        port = atoi(optarg);
                        break;
                  case 's':
                        snd_port = atoi(optarg);
                        break;
		  case 't':
			timer_interval = atoi(optarg);
			break;
		  case 'v':
			do_logging = TRUE; 
			break;
                  default:
                        printf("Usage: %s -a [dest IP] ", argv[0]);
			printf(" -p [receive port] -s [send port]");
			printf(" -m [message length] -n [number of sends]");
			printf(" -t timer interval\n");
                        exit(1);
                }
        }

	if (clock_getres(CLOCK_REALTIME,&tm_spec) == -1) {
	      perror("clock get resolution");
	      exit(1);
	}
	printf("Resolution is %ld micro seconds.\n",tm_spec.tv_nsec/1000);

	sd = udp_unicast();
	if ((rd_sd = udp_allow_all(port)) < 0) {
		printf("%s: receive socket create or bind failed\n", argv[0]);
                exit(1);
	}

	memset(&snd_addr, 0, sizeof(snd_addr));
	snd_addr.sin_family = AF_INET;
	snd_addr.sin_addr.s_addr = inet_addr(snd_ip);
	snd_addr.sin_port = htons(snd_port);

        printf("send address to ip 0x%x, port %d initialized\n",
                        ntohl(snd_addr.sin_addr.s_addr),
                        ntohs(snd_addr.sin_port));
	timeout.tv_sec = timer_interval/1000;
	timeout.tv_usec = (timer_interval%1000) * 1000;
	get_current_timestamp(&last_ts);

	for (i = 0; i < num_sends || num_sends == 0; i++) {
		int time_left;
		int time_used;

		FD_ZERO(&readfds);
		FD_SET(rd_sd, &readfds);

		ndesc = select(rd_sd+1, &readfds, NULL, NULL, &timeout);
		
		if ((ndesc > 0) && FD_ISSET(rd_sd, &readfds)) {
			// read socket and compute skew 
			skew_estimate = read_socket_get_skew(
						rd_sd,
						&rcv_addr,
						do_logging);
		}

	        if (ndesc < 0) {
			perror("Bad select");
		}
		// portable code so no assumptions about value of timeout here
		get_current_timestamp(&current_ts);
		time_used = TS_TO_MS(&current_ts) - TS_TO_MS(&last_ts);
		time_left = timer_interval - time_used;
		if (time_left <= 0) {
			send_time_to_remote(
				sd,
				&snd_addr,
				skew_estimate,
				message_length);
			if (do_logging)
				printf("sent time: skew %.6f time_left %d\n",
					skew_estimate, time_left);
			time_left = timer_interval;
			last_ts = current_ts;
		}	
			
		timeout.tv_sec = time_left/1000;
		timeout.tv_usec = (time_left%1000) * 1000;
	}
	return 0;
}

/** Assumes message format from remote system:
 *	 id, length, start sec, start nsec, remote sec, remote nsec
 *  (Better to add recognizable time message type field?)
 *
 *  Returns skew estimate; a positive skew means the time at the
 *  remote station running clock_set is later than the time at the
 *  receiving station running clock_snd_skew
 */
double read_socket_get_skew(int sd, struct sockaddr_in *paddr,
			int do_logging)
{
	int msg_id;	// read from message received
	int msg_type;	
	int msg_len;
	struct timespec cur_tm;
	struct timespec start_tm;
	struct timespec rmt_tm;
	struct timespec trip_out_tm;
	struct timespec trip_back_tm;
	int trip_out_sign, trip_back_sign;
	struct timespec trip;
	double delta;		// half of round trip time
	double trip_out, trip_back;
	double ave_delta;	// average over all trips
	double clock_skew1;	// estimate based on forward trip
	double clock_skew2;	// estimate based on return trip
	double skew;		// average of two estimates
	static int num_stat = 0;
	static double total_delta = 0;
	static char rcv_buffer[RBUFSIZE];
	int bytes_received;
#ifdef WAVE
	sWAVE_Rx_Option rx_options;	// not used
#else
	int rx_options;	//dummy
#endif
	int rx_option_length = 0;

	int socklen = sizeof(struct sockaddr_in);
	int retval;
	int num_items;

	memset(paddr, 0, socklen);
	if ((bytes_received = recvfrom(sd, 
			rcv_buffer, RBUFSIZE-1, 0, 
			(struct sockaddr *) paddr,
			&socklen))  <= 0) {
		perror("recvfrom failed");
		return 0.0;
	}	

	if (clock_gettime(CLOCK_REALTIME,&cur_tm) == -1 ) {
		perror("clock gettime");
	}

	num_items = sscanf(rcv_buffer,"%d %d %d %d %ld %d %ld",
		&msg_id, &msg_type, &msg_len, 
		&start_tm.tv_sec, &start_tm.tv_nsec,
		&rmt_tm.tv_sec, &rmt_tm.tv_nsec);
	if (do_logging)
		printf("num_items %d msg_type %d\n", num_items,
			msg_type);

	// return with 0.0 for skew estimate if bad message decode
	if (num_items != 7 || msg_type != CLOCK_SKW_MSG_TYPE) {
		printf("sscanf error, num items %d, msg type %d\n",
			num_items, msg_type);
		return 0.0;
	}

	trip_out_sign = diff_timespec(rmt_tm, start_tm, &trip_out_tm);
	trip_back_sign = diff_timespec(cur_tm, rmt_tm, &trip_back_tm);
	
	trip_out = timespec2double(trip_out_tm);
	if (trip_out < ave_delta) {	// both must always be non-negative
		skew = 0.0;
		return skew;	// to avoid nonsense when clock is just reset
	}
	if (trip_out_sign) trip_out = -trip_out;

	trip_back = timespec2double(trip_back_tm);
	if (trip_back_sign) trip_back = -trip_back;

	// will be non-negative because clocks on same system
	(void) diff_timespec(cur_tm, start_tm, &trip); //round trip

	delta = timespec2double(trip)/2.0;
	if (delta < 0.0) delta = 0.0;

	num_stat++;
	total_delta += delta;	 
	ave_delta = total_delta/num_stat;	//average time one-way

	
	clock_skew1 = trip_out-ave_delta; // amount remote is later than local 

	clock_skew2 = ave_delta-trip_back; 

	skew = (fabs(clock_skew1) +fabs(clock_skew2))/2.0;

	if (trip_out_sign)	// 1 means negative, 0 positive
		skew = -skew;

	// times in seconds, to millisecond resolution
	// seconds part displayed mod (24*3600)
	if (do_logging) {
                printf("%d\t%d %u.%03lu %u.%03lu %u.%03lu ", msg_id, msg_len,
			start_tm.tv_sec % (24*3600), start_tm.tv_nsec/1000000,
			rmt_tm.tv_sec % (24*3600), rmt_tm.tv_nsec/1000000,
			cur_tm.tv_sec % (24*3600), cur_tm.tv_nsec/1000000);

                printf("%.3f %.3f %.6f\n", delta, ave_delta, skew);
		fflush(stdout);
	}
	return skew;

}

/**
 * 	Sends the current time and the current estimate of the clock
 *      skew between here and the destination.
 */
void send_time_to_remote(int sd, struct sockaddr_in *paddr, double skew,
			int message_length)
{
		struct timespec tm_spec;
		static char snd_buffer[RBUFSIZE];
		static int msg_id = 0;
		int bytes_sent = 0;	// returned from sendto 

		if (clock_gettime(CLOCK_REALTIME,&tm_spec) == -1 ) {
			 perror("clock gettime");
			 exit(1);
		}
		sprintf(snd_buffer, "%d %d %u %lu %.6f", 
					msg_id++, 
					CLOCK_SET_MSG_TYPE,
					tm_spec.tv_sec, 
					tm_spec.tv_nsec,
					skew); 

		if (message_length > 0) {
			int min = strlen(snd_buffer);
			int j;

			for (j = min;
				j < message_length-1 && j < RBUFSIZE-1; j++) {
				// insert space after formatted string
				if (j == min) 
					snd_buffer[j] = ' ';
				else
					snd_buffer[j] = j % 10 + '0';
			}
			snd_buffer[j] = '\0'; //mark end of string
		}

		bytes_sent = sendto(sd, snd_buffer, strlen(snd_buffer)+1, 0,
				(struct sockaddr *) paddr,
				sizeof(struct sockaddr_in)); 
#ifdef DO_TRACE
		printf("sent %d bytes, strlen %d\n", bytes_sent,
					 strlen(snd_buffer)+1);
#endif
}
