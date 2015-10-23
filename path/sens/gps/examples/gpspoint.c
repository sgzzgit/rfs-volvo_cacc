/**\file 
 *
 * gpspoint.c   Read GPS data from stdin and writes "GPS points"
 *		one on each line of stdout. 
 *
 *		Can use gpsd by piping tcp_client -q r to stdin. 
 * 		Writes point whenever trigger message is received.
 *
 *  Copyright (c) 2008   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timestamp.h>
#include <db_clt.h>
#include <db_utils.h>
#include "path_gps_lib.h"

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

#define MAX_BUFFERED_GPS_DATA	20

int main(int argc, char *argv[])
{
	int status;
	char sentence[MAX_GPS_SENTENCE];
	gps_data_typ gps_data[MAX_BUFFERED_GPS_DATA];
	timestamp_t ts;
	int verbose = 0;	/// if 1, print extra info for debugging
	int option;
	path_gps_point_t hb;	/// fill in from GPS messages received
	char *trigger_message = "GPRMC"; 
	int num_buffered = 0;
	int sequence_no = 0;

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "t:v")) != EOF) {
		switch(option) {
		case 't':
			trigger_message = strdup(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -t  (trigger message)  \n ");
			fprintf(stderr, " -v  (verbose) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (setjmp(env) != 0) {
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	memset(&hb, 0, sizeof(hb));
	while (1) {
		/// Blocks until it gets a good GPS sentence
		path_gps_get_sentence(sentence, stdin, 0, 0); 
		path_gps_parse_sentence(sentence, &gps_data[num_buffered]);
		if (verbose) {
			path_gps_print_data(stdout, &gps_data[num_buffered]);
			printf("\n");
		}

		num_buffered++;
		if  (strcmp(gps_data[num_buffered - 1].gps_id, trigger_message) == 0 ||
				num_buffered == MAX_BUFFERED_GPS_DATA) {
			path_gps_get_point(gps_data, num_buffered, &hb);	
			num_buffered = 0;
			hb.sequence_no = sequence_no++;
			path_gps_print_point(stdout, &hb);
			printf("\n");
		}
			
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
