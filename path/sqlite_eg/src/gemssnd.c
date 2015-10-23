/**\file 
 *
 * gpssnd.c     Read GPS data from stdin and write to an sqlite table 
 *
 *		Reads three message types, if present:
 *		GGA, VTG and RMC 
 *
 *		Writes to  one GPS point summary table formatted as is usual
 *		at PATH; later will move to the J2735 Basic Safety Message.
 *		
 *		Broadcasts GPS point as UDP message to neighbors and optionally
 *		also broadcasts the same UDP message to RSU for TSP on a
 *		different port.
 *
 *		Sends are combined with sqlite3 table insert to avoid
 *		extra load on reading the table by a separate send process.
 *
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
#include <udp_utils.h>
#include <sqlite3.h>
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

/**
 *	callback function, used with query returning statements
 *	to handle results; no results from queries yet in this
 *	program
 */
static int callback(void *not_used, int argc, char **argv, char **az_col){
	return 0;
}

static void sqlt3_ex(sqlite3 *sqlt, char *cmd_str)
{
	char *zerr = NULL;
	if (sqlite3_exec(sqlt, cmd_str, callback, 0, &zerr) != SQLITE_OK) {
			fprintf(stderr, "sqlite3 error: %s\n", zerr);
			sqlite3_free(zerr);
	}  
}

int main(int argc, char *argv[])
{
	int status;
	char sentence[MAX_GPS_SENTENCE];
	gps_data_typ gps_data;
	timestamp_t ts;
	int verbose = 0;	/// if 1, print extra info for debugging	
	int do_timing = 0;	/// if 1, print user and system time 
	int do_raw = 0;		/// if 1, keep tables of raw messages
	struct avcs_timing timing;
	int option;

        char *udp_name = "192.168.255.255";     /// default broadcast
	char *second_udp_name = "192.168.255.255";	
        int udp_port = 7015;
	int second_udp_port = 0;	/// must set for second send
        int sd_out;             /// socket descriptor for UDP send
        int second_sd_out;      /// socket descriptor for UDP send
        int bytes_sent;         /// returned from sendto
        struct sockaddr_in snd_addr;  		/// used in sendto call
        int do_broadcast = 1;   /// by default, broadcast socket
        int do_second_broadcast = 1;   /// by default, broadcast socket

	path_gps_point_t pt;	/// fill in from GPS messages received
	sqlite3 *sqlt;
	char *sqlt_name = "my_gps.db";/// default name for sqlite database
	char *trigger_msg = "GPRMC";	/// NEMA ID to trigger heartbeat insert
	char cmd_str[MAX_CMD_STR];	/// pass to sqlite3_exec

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "a:b:d:gm:n:p:r:tv")) != EOF) {
		switch(option) {
                case 'a':
                        udp_name = strdup(optarg);
                        break;
                case 'b':
                        second_udp_name = strdup(optarg);
                        break;
		case 'd':
			sqlt_name = strdup(optarg);
			break;
		case 'g':
			do_raw = 1;	/// puts raw GPS in table 
			break;
		case 'm':
			trigger_msg = strdup(optarg);
			break;
		case 'n':
			switch (atoi(optarg)) {
			case 1:
				do_broadcast = 0;
				break;
			case 2:
				do_second_broadcast = 0;
				break;
			case 3:
				do_second_broadcast = 0;
				do_broadcast = 0;
				break;
			case 0:
			default:
				break;	// do broadcast
			}
			break;
		case 'p':
			udp_port = atoi(optarg);
			break;
		case 'r':
			second_udp_port = atoi(optarg);
			break;
		case 't':
			do_timing = 1;	/// prints sys and user time
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -a  <UDP address> \n ");
			fprintf(stderr, " -b  <second UDP address> \n ");
			fprintf(stderr, " -d  <database name> \n ");
			fprintf(stderr, " -m  <trigger msg NEMA ID> \n ");
			fprintf(stderr, " -n  <default: 0 both broadcast> \n ");
			fprintf(stderr, " \t1 first unicast, 2 second unicast");
			fprintf(stderr, " \t3 both unicast\n");
			fprintf(stderr, " -g  (put raw GPS in table) \n ");
			fprintf(stderr, " -p  <UDP port> \n ");
			fprintf(stderr, " -r  <second UDP port> \n ");
			fprintf(stderr, " -t  (measure timing) \n ");
			fprintf(stderr, " -v  (verbose mode) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (do_timing)
		avcs_start_timing(&timing);

        if (do_broadcast) {
                sd_out = udp_broadcast();
        } else
                sd_out = udp_unicast();
        if (sd_out < 0) {
                printf("failure opening socket on %s %p\n",
                                 udp_name, udp_port);
	}

	if (second_udp_port) {
		if (do_second_broadcast) {
			second_sd_out = udp_broadcast();
		} else
			second_sd_out = udp_unicast();

		if (sd_out < 0) {
			printf("failure opening socket on %s %p\n",
					 second_udp_name, second_udp_port);
		}
	}


	if (sqlite3_open(sqlt_name, &sqlt) != 0) {
		fprintf(stderr, "Can't open %s\n", sqlite3_errmsg(sqlt));
		sqlite3_close(sqlt);
		exit(EXIT_FAILURE);
	}
		
	if (setjmp(env) != 0) {
		if (do_timing) {
			printf("Exiting %s\n", argv[0]);
			fflush(stdout);
			avcs_end_timing(&timing);
			avcs_print_timing(stdout, &timing);
			fflush(stdout);
		}
		sqlite3_close(sqlt);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	memset(&pt, 0, sizeof(pt));

	while (1) {
		int do_insert = 0;	// only insert for some GPS messages
		/// Blocks until it gets a good GPS sentence
		timing.exec_num++;
		path_gps_get_sentence(sentence, stdin, 0, 0); 
		path_gps_parse_sentence(sentence, &gps_data);
		if (verbose) {
			path_gps_print_data(stdout, &gps_data);
			printf("\n");
		}
		if (strcmp(gps_data.gps_id, "GPGGA") == 0) {                    
                        gps_gga_typ *pgga = &gps_data.data.gga;                 
                        get_current_timestamp(&pt.local_time);
                        pt.utc_time = gpsutc2ts(pgga->utc_time);
                        pt.latitude = path_gps2degree(pgga->latitude);          
                        pt.longitude = path_gps2degree(pgga->longitude);        
                        pt.pos = pgga->pos;                                     
                        pt.num_sats = pgga->num_sats;
			do_insert = 1;
		} else if (strcmp(gps_data.gps_id, "GPRMC") == 0) {
			gps_rmc_typ *prmc = &gps_data.data.rmc;
			get_current_timestamp(&pt.local_time);
			pt.utc_time = gpsutc2ts(prmc->utc_time);	
			pt.latitude = path_gps2degree(prmc->latitude); 
			pt.longitude = path_gps2degree(prmc->longitude); 
			pt.speed = path_knots2mps(prmc->speed_knots);
			pt.heading = prmc->true_track;
			do_insert = 1;
		} else if (strcmp(gps_data.gps_id, "GPVTG") == 0) { 
			gps_vtg_typ *pvtg = &gps_data.data.vtg;
			get_current_timestamp(&pt.local_time);
			pt.speed = path_knots2mps(pvtg->speed_knots);
			pt.heading = pvtg->true_track;
			do_insert = 1;
		}	
		
		/// insert current GPS message into appropriate table
		if (do_raw && do_insert) {
			path_gps_data_insert_str(cmd_str, &gps_data);
			if (verbose)
				printf("path_gps_data_insert_str: %s\n",
					 cmd_str);
			sqlt3_ex(sqlt, cmd_str);
		}

		/// insert GPS points in table when trigger message arrives
		if  (strcmp(gps_data.gps_id, trigger_msg) == 0) {
			pt.sequence_no++;
			// if wraparound to <0, set positive
			if (pt.sequence_no <= 0) pt.sequence_no = 1;
			if (verbose) {
				path_gps_print_point(stdout, &pt);
				printf("\n");
			}
			path_gps_point_insert_str(cmd_str, &pt);
			if (verbose)
				printf("path_gps_point_insert_str: %s\n",
					 cmd_str);
			sqlt3_ex(sqlt, cmd_str);

		}

		/// Send UDP message(s) with GPS information
		set_inet_addr(&snd_addr, inet_addr(udp_name), udp_port);
		bytes_sent = sendto(sd_out, &pt, sizeof(pt), 0,
                                        (struct sockaddr *) &snd_addr,
                                        sizeof(snd_addr));
		if (second_udp_port) {
			set_inet_addr(&snd_addr, inet_addr(second_udp_name),
				 second_udp_port);
			bytes_sent = sendto(second_sd_out, &pt,
					sizeof(pt), 0,
                                        (struct sockaddr *) &snd_addr,
                                        sizeof(snd_addr));
		}

	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
