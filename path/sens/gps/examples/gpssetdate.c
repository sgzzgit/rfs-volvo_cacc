/**\file 
 *
 * gpssnd.c     Read GPS data and set the date and time on the local system. 
 *
 *		Reads stdin for NEMA stream.
 *
 *		Set system clock to match GPS on first RMC message,
 *		because embedded systems often lose their date memory.
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
#include <timing.h>
#include <timestamp.h>
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
	int verbose = 0;	/// if 1, print extra info for debugging	
	path_gps_point_t hb;	/// fill in from GPS messages received
	int option;
	char datetime_str[30];
	int hour;

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "v")) != EOF) {
		switch(option) {
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -v  (verbose) \n ");
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
		path_gps_get_sentence(sentence, stdin, 0, -1); 
		path_gps_parse_sentence(sentence, &gps_data);
		if (verbose) {
			path_gps_print_data(stdout, &gps_data);
			printf("\n");
		}
		if (strcmp(gps_data.gps_id, "GPRMC") == 0) {
			gps_rmc_typ *prmc = &gps_data.data.rmc;
			get_current_timestamp(&hb.local_time);
			hb.utc_time = gpsutc2ts(prmc->utc_time);	
			hb.latitude = path_gps2degree(prmc->latitude); 
			hb.longitude = path_gps2degree(prmc->longitude); 
			hb.speed = path_knots2mps(prmc->speed_knots);
			hb.heading = prmc->true_track;
			hour = ((int)(prmc->utc_time/10000)%100);
			hour = hour < 7 ? (hour + 17) : (hour-7);
			sprintf(datetime_str, "date %04d%02d%02d%02d%02d.%02d",
				2000 + (prmc->date_of_fix%100),
				(prmc->date_of_fix/100)%100,
				(prmc->date_of_fix/10000)%100,
				hour,
				((int)(prmc->utc_time/100)%100),
				((int)prmc->utc_time%100)
				);
//date 120515, time 222910.000
			if (verbose)
				printf("date %d, time %.3f\n", 
					    prmc->date_of_fix, prmc->utc_time);
				printf("datestr %s\n", datetime_str);
//			clockset2gps(prmc->date_of_fix,prmc->utc_time);	
			system(datetime_str);
			break;	// exit after setting clock
		} 
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
