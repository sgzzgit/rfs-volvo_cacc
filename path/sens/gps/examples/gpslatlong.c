/**\file 
 *
 * gpslatlong.c  Read GPS RMC or GGA and writes utc,lat,long to stdout 
 *
 * 		 Reads GPS stream from stdin. 
 *
 * 		 Usage:
 * 		 cat </dev/ttyUSB0 | gpsgpslatlong |...
 * 		 or (on a system where gpsd is running) 
 *		 tcp_client -q r | gpsgpslatlong | 
 *		  
 *		 utc time is in seconds since midnight, not
 *		 hhmmss.sss as it comes in the NEMA message
 *		 latitude and longitude are double float degrees
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

#define MAX_SENTENCE	256

int main(int argc, char *argv[])
{
	int status;
	char sentence[MAX_GPS_SENTENCE];
	gps_data_typ gps_data;
	timestamp_t ts;
	int verbose = 0;	/// print extra info for debugging	
	int use_rmc = 0;	/// by default use GGA messages
	int option;

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "m:d:f:gp:v")) != EOF) {
		switch(option) {
		case 'v':
			verbose = 1;
			break;
		case 'r':
			use_rmc = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: ", argv[0]); 
			fprintf(stderr, " -v  (verbose) ");
			fprintf(stderr, " -r  (use_rmc) ");
			fprintf(stderr, "\n");
  		 	fprintf(stderr, "cat </dev/ttyUSB0 | gpsgpslatlong \n");
  		 	fprintf(stderr, "or (on a system with gpsd\n");
 		 	fprintf(stderr, "tcp_client -q r | gpsgpslatlong \n"); 
			exit(EXIT_FAILURE);
		}
	}
	if (setjmp(env) != 0) {
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	while (1) {
		/// Blocks until it gets a good GPS sentence
		path_gps_get_sentence(sentence, stdin, 0, -1); 
		path_gps_parse_sentence(sentence, &gps_data);
		if (verbose) {
			path_gps_print_data(stdout, &gps_data);
			printf("\n");
		}

		/// Print if message is from chosens message type, RMC default
		if (strcmp(gps_data.gps_id, "GPGGA") == 0){
			timestamp_t ts = gpsutc2ts(gps_data.data.gga.utc_time);
			printf("%.3lf %.6lf %.6lf %d\n", 
				TS_TO_SEC(&ts),
				path_gps2degree(gps_data.data.gga.latitude),
				path_gps2degree(gps_data.data.gga.longitude),
				gps_data.data.gga.num_sats);
		} else if (use_rmc && (strcmp(gps_data.gps_id, "GPRMC") == 0)){ 
			timestamp_t ts = gpsutc2ts(gps_data.data.rmc.utc_time);
			printf("%.3lf %.6lf %.6lf\n", 
				TS_TO_SEC(&ts),
				path_gps2degree(gps_data.data.rmc.latitude),
				path_gps2degree(gps_data.data.rmc.longitude));
		}	
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
