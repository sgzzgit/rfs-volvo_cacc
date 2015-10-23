/**\file 
 *
 * tsprcv.c	Read GPS point message and write to BUS_INPUT_TYPE
 *
 *		For now expects path_gps_point_t as body of message. 
 *		Later create a J2735 structure.
 *
 *  Copyright (c) 2008   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 * Kun June 25, 2008 added timeb structure for debugging the communication delay 
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timestamp.h>
#include <timing.h>
#include <udp_utils.h>
#include <db_clt.h>
#include <db_utils.h>
#include "path_gps_lib.h"

static jmp_buf exit_env;
static int sig_list[] = {
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,
        ERROR,
};
static void sig_hand(int code)
{
        if (code == SIGALRM)
                return;
        else
                longjmp(exit_env, code);
}


// THIS MUST MATCH  definitions in /home/tsp
#define DB_BUS01_INPUT_TYPE 7201

// Following is data structure for DB_BUS01_INPUT_TYPE
typedef struct
{
        int   bus_ID;
        float speed,long_accel,door_status;
	float latitude, longitude, utc_time, speed_over_ground;
        char  hour,min,sec;
        short int millisec;
} bus_input_typ;

static db_id_t db_vars[] = {
        {DB_BUS01_INPUT_TYPE, sizeof(bus_input_typ)},
};

#define NUM_DB_VARS     sizeof(db_vars)/sizeof(db_id_t)


// TSP people use raw lat, long values from NMEA stream, don't set negative
void convert_back_for_tsp(double latitude, double longitude,
		 float *ptsp_lat, float *ptsp_long)
{
	int i_lat;
	int i_long;
	float tsp_lat;
	float tsp_long;

	longitude = fabs(longitude);
	i_lat = latitude;
	i_long = longitude;
	tsp_lat = (latitude - i_lat) * 60.0 + 100.0 * i_lat;
	tsp_long = (longitude - i_long) * 60.0 + 100.0 * i_long;
	*ptsp_lat = tsp_lat;
	*ptsp_long = tsp_long;
}

int main(int argc, char *argv[])
{
	int status;
	
        char hostname[MAXHOSTNAMELEN];
        char *domain = DEFAULT_SERVICE; /// on Linux sets DB q-file directory
        db_clt_typ *pclt;       /// data server client pointer
        int xport = COMM_PSX_XPORT;     /// Linux uses Posix message queues
	int do_create = 0;	/// set do_create for standalone test
	db_id_t *pdb_vars = NULL;	/// set to db_vars for standalone test
	int num_db_vars = 0;	/// set to NUM_DB_VARS for standalone test

	int verbose = 0;	/// if 1, print extra info for debugging	
	int do_timing = 0;	/// if 1, print users and sys time at end 
	struct avcs_timing timing;	
	int udp_port = 7015;	/// port for receiving heartbeat
	int option;

	path_gps_point_t pt;	/// fill in from GPS messages received
	bus_input_typ bus_input;	/// fill in from pt
	int sd_in;		/// socket descriptor for UDP send
	int bytes_rcvd;		/// returned from recvfrom
	struct sockaddr_in src_addr;	/// used in recvfrom call
	unsigned int socklen;

	struct timeb timeptr_raw;
	struct tm time_converted;

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "cvtu:")) != EOF) {
		switch(option) {
		case 'c':
			do_create = 1;	// call timing utilities	
			break;
		case 't':
			do_timing = 1;	// call timing utilities	
			break;
		case 'v':
			verbose = 1;	//
			break;
		case 'u':
			udp_port = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Usage %s: ", argv[0]); 
			fprintf(stderr, " -f <prefix>");
			fprintf(stderr, " -v  (verbose) ");
			fprintf(stderr, " -u  (UDP port number for input) ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
		}
	}
	sd_in = udp_allow_all(udp_port);
	if (sd_in < 0) {
		printf("failure opening socket on %d\n", udp_port);
		exit(EXIT_FAILURE);
	}
	if (do_create) {
		pdb_vars = &db_vars[0];
		num_db_vars = NUM_DB_VARS;
	}

	get_local_name(hostname, MAXHOSTNAMELEN);
	if (( pclt = db_list_init( argv[0], hostname, domain, xport,
                        pdb_vars, num_db_vars, NULL, 0)) == NULL ) {
                        printf("Database initialization error in %s\n", argv[0]);
                        exit(EXIT_FAILURE);
	}

	if (setjmp(exit_env) != 0) {
		if (do_timing) {
			printf("Exiting gpsrcv\n");
			fflush(stdout);
			avcs_end_timing(&timing);
			avcs_print_timing(stdout, &timing);
			fflush(stdout);
		}
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	socklen = sizeof(src_addr);
	memset(&src_addr, 0, socklen);

	if (do_timing) 
		avcs_start_timing (&timing);
	while (1) {
		timestamp_t ts;
		timing.exec_num++;
		bytes_rcvd = recvfrom(sd_in, &pt, sizeof(pt), 0,
				(struct sockaddr *) &src_addr, &socklen);

		if (bytes_rcvd < 0) {
			perror("recvfrom failed\n");
			continue;
		}
		
		ftime ( &timeptr_raw );
		localtime_r ( &timeptr_raw.time, &time_converted );
		memset(&bus_input, 0, sizeof(bus_input));
		bus_input.bus_ID = 1;
		bus_input.utc_time = ts2gpsutc(&pt.utc_time);
//		bus_input.hour = pt.local_time.hour;
//		bus_input.min = pt.local_time.min;
//		bus_input.sec = pt.local_time.sec;
//		bus_input.millisec = pt.local_time.millisec;
		bus_input.hour = (char)time_converted.tm_hour;
		bus_input.min = (char)time_converted.tm_min;
		bus_input.sec = (char)time_converted.tm_sec;
		bus_input.millisec = timeptr_raw.millitm;
		bus_input.speed_over_ground = pt.speed;
		bus_input.speed = pt.speed;

		convert_back_for_tsp(pt.latitude, pt.longitude, 
				&bus_input.latitude, &bus_input.longitude);

		db_clt_write(pclt, DB_BUS01_INPUT_TYPE,
			 sizeof(bus_input_typ), &bus_input);

		if (verbose) {
			fprintf(stdout, " 0x%08x ", src_addr.sin_addr.s_addr);
			path_gps_print_point(stdout, &pt);
			fprintf(stdout, "\n");
			fprintf(stdout,
				"%02d:%02d:%02d.%03d %02d:%02d:%02d.%03d %.6f %.6f %.3f %.3f", 
				bus_input.hour,
				bus_input.min,
				bus_input.sec,
				bus_input.millisec,
				pt.local_time.hour,
				pt.local_time.min,
				pt.local_time.sec,
				pt.local_time.millisec,
				bus_input.latitude,
				bus_input.longitude,
				bus_input.utc_time,
				bus_input.speed);
			fprintf(stdout, "\n");
		}
	}
}

