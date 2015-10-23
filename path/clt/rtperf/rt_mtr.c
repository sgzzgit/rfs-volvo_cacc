/**\file
 *	Monitor round trip time, gets reply to message sent from
 *	another process on same system.
 *	Uses UDP sockets. 
 */

#include <sys_os.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <local.h>
#include <sys_rt.h>
#include <udp_utils.h>
#include <timestamp.h>
#include "clock_set.h"

int main(int argc, char **argv)
{
	int sd = -1;
	struct sockaddr_in rcv_addr;
	unsigned int origin;
	char rcv_buffer[RBUFSIZE];	
	int bytes_received;	// received from a call to recv 
	int num_stat = 0;	// number of receives 
	int num_items;		// returned from sscanf
	int opt;
	int verbose = 0;
	double total_delta = 0.0;
	int port = CLOCK_MONITOR_PORT;

	while ((opt = getopt(argc, argv, "p:v")) != -1) {
                switch (opt) {
		case 'p':
			port = atoi(optarg);;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage: %s -p [port]", argv[0]);
			printf(" -r [0 to set time, 1 for round trip]\n");
			exit(1);
                }
        }

	if ((sd = udp_allow_all(port)) < 0) exit(1);
	fprintf(stderr, "%s receiving on socket %d, port %d\n", argv[0],
		       		sd, port);

	for (;;) {
		int msg_id;
		int msg_type;
		int msg_len;
		timestamp_t path_ts;
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

		int socklen = sizeof(rcv_addr);
		int retval;
#ifdef WAVE
		sWAVE_Rx_Option rx_options;
#else
		int rx_options;
#endif
		int rx_option_length = sizeof(rx_options);
		//mtr_rssi is the one received here
		//rmt_rssi is the one sent from clock_rcv on the remote system
		short rmt_rssi, mtr_rssi;

		memset(&rcv_addr, 0, socklen);
		if ((bytes_received = recvfrom(sd, 
				rcv_buffer, RBUFSIZE-1, 0, 
				(struct sockaddr *) &rcv_addr,
				&socklen))
					  <= 0) {
			if (verbose)
				perror("recvfrom failed");
			// usually ignore, probably due to channel switch
			continue;
		}	

		if (clock_gettime(CLOCK_REALTIME,&cur_tm) == -1 ) {
			perror("clock gettime");
		}
		origin = ntohl(rcv_addr.sin_addr.s_addr);

		num_items = sscanf(rcv_buffer,"%d %d %d %d %ld %d %ld ", 
			&msg_id, &msg_type, &msg_len,  
			&start_tm.tv_sec,&start_tm.tv_nsec,
			&rmt_tm.tv_sec,&rmt_tm.tv_nsec);

		if (verbose) {
			// cut off end of string before printing
			rcv_buffer[64] = 0;
			printf("rcv_buffer %s\n", rcv_buffer);
		}
		// ignore message if sscanf decode is incorrect
		if (msg_type != CLOCK_MTR_MSG_TYPE) {
			if (verbose) {
				// Usually, quietly ignore, since you get
				// all raw IP packets including those
				// not of the correct type
				printf("bad decode: num_items %d msg_type %d\n",
					num_items, msg_type);
			}
			continue;
		}

		trip_out_sign = diff_timespec(rmt_tm, start_tm, &trip_out_tm);
		trip_back_sign = diff_timespec(cur_tm, rmt_tm, &trip_back_tm);
		
		trip_out = timespec2double(trip_out_tm);
		if (trip_out_sign) trip_out = -trip_out;

		trip_back = timespec2double(trip_back_tm);
		if (trip_back_sign) trip_back = -trip_back;

		// will be non-negative because clocks on same system
		(void) diff_timespec(cur_tm, start_tm, &trip);

		delta = timespec2double(trip)/2.0;

		num_stat++;
		total_delta += delta; 
		ave_delta = total_delta/num_stat;
		clock_skew1 = trip_out-delta; 
		clock_skew2 = delta-trip_back; 
		get_current_timestamp(&path_ts);
		print_timestamp(stdout, &path_ts);
		printf("\t");

		print_ip_address(origin);

		// times in seconds, to millisecond resolution
		// seconds part displayed mod 100000
                printf("%d %d %u.%03lu %u.%03lu %u.%03lu ", msg_id, msg_len,
			start_tm.tv_sec % 100000, start_tm.tv_nsec/1000000,
			rmt_tm.tv_sec % 100000, rmt_tm.tv_nsec/1000000,
			cur_tm.tv_sec % 100000, cur_tm.tv_nsec/1000000);

                printf("%.3f %.3f %.3f \n", 
			delta, ave_delta,
			(fabs(clock_skew1) +fabs(clock_skew2))/2.0);

		fflush(stdout);

	}
	return 0;
}
