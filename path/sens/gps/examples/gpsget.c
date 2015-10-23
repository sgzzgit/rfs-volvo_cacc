/**\file 
 *
 * gpsget.c     Read GPS data from stdin and writes "GPS points"
 *		one on each line of stdout. 
 *
 *		Reads three message types, if present:
 *		GGA, VTG and RMC 
 *
 *		Can use gpsd by piping tcp_client -q r to stdin. 
 * 		Writes point whenever RMC message is received.
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

int main(int argc, char *argv[])
{
	int status;
	char sentence[MAX_GPS_SENTENCE];
	gps_data_typ gps_data;
	timestamp_t ts;
	int verbose = 0;	/// if 1, print extra info for debugging
	int option;
	path_gps_point_t hb;	/// fill in from GPS messages received

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "v")) != EOF) {
		switch(option) {
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -t  (do timing) \n ");
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
		path_gps_parse_sentence(sentence, &gps_data);
		if (verbose) {
			path_gps_print_data(stdout, &gps_data);
			printf("\n");
		}
		if (strcmp(gps_data.gps_id, "GPGGA") == 0) {                    
                        gps_gga_typ *pgga = &gps_data.data.gga;                 
                        get_current_timestamp(&hb.local_time);
                        hb.utc_time = gpsutc2ts(pgga->utc_time);
                        hb.latitude = path_gps2degree(pgga->latitude);          
                        hb.longitude = path_gps2degree(pgga->longitude);        
                        hb.pos = pgga->pos;                                     
                        hb.num_sats = pgga->num_sats;
                        hb.hdop = pgga->hdop;
		} else if (strcmp(gps_data.gps_id, "GPRMC") == 0) {
			gps_rmc_typ *prmc = &gps_data.data.rmc;
			get_current_timestamp(&hb.local_time);
			hb.utc_time = gpsutc2ts(prmc->utc_time);	
			hb.latitude = path_gps2degree(prmc->latitude); 
			hb.longitude = path_gps2degree(prmc->longitude); 
			hb.speed = path_knots2mps(prmc->speed_knots);
			hb.heading = prmc->true_track;
		} else if (strcmp(gps_data.gps_id, "GPVTG") == 0) { 
			gps_vtg_typ *pvtg = &gps_data.data.vtg;
			get_current_timestamp(&hb.local_time);
			hb.speed = path_knots2mps(pvtg->speed_knots);
			hb.heading = pvtg->true_track;
		}	

		if  (strcmp(gps_data.gps_id, "GPRMC") == 0) {
				path_gps_print_point(stdout, &hb);
				printf("\n");
		}
			
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
