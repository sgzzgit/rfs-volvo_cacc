/**\file 
 *
 * ptrcv.c      Read heartbeat message and write to an sqlite3 database.
 *
 *		For now expects path_gps_point_t as body of message. 
 *		Later create a J2735 structure.
 *
 *		Records local timestamp when received as well as 
 *		UTC time and local timestamp when sent from the message.	
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
#include <timing.h>
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
 *      callback function, used with query returning statements
 *      to handle results; no results from queries yet in this
 *      program
 */
static int callback(void *not_used, int argc, char **argv, char **az_col){
        return 0;
}

/** 
 *	Convenience wrapper for sqlite3_exec that does error checking.
 */
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
	
	int verbose = 0;	/// if 1, print extra info for debugging	
	int do_timing = 0;	/// if 1, print users and sys time at end 
	struct avcs_timing timing;	
	int udp_port = 7015;	/// port for receiving heartbeat
	int option;

	path_gps_point_t pt;	/// fill in from GPS messages received
	int sd_in;		/// socket descriptor for UDP receive
	int bytes_rcvd;		/// returned from recvfrom
	struct sockaddr_in src_addr;	/// used in recvfrom call
	unsigned int socklen;
        sqlite3 *sqlt;
        char *sqlt_name = "other_gps.db";/// default name for sqlite database
	char cmd_str[MAX_CMD_STR];

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "d:vtu:")) != EOF) {
		switch(option) {
		case 'd':
			sqlt_name = strdup(optarg);	
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

        if (sqlite3_open(sqlt_name, &sqlt) != 0) {
                fprintf(stderr, "Can't open %s\n", sqlite3_errmsg(sqlt));
                sqlite3_close(sqlt);
                exit(EXIT_FAILURE);
        }

	if (setjmp(env) != 0) {
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

		/// copy source IP address into object_id field 
		{
			int i;
			unsigned char *p = (unsigned char *)
						 &src_addr.sin_addr.s_addr;
			memset(&pt.object_id, 0, GPS_OBJECT_ID_SIZE);
			for (i = 0; i < 4; i++) 
				pt.object_id[i] = p[i];
		}

		if (verbose) {
			path_gps_print_point(stdout, &pt);
			fprintf(stdout, "\n");
		}
		path_gps_point_insert_str(cmd_str, &pt);
		if (verbose)
			printf("path_gps_point_insert_str: %s\n", cmd_str);
		sqlt3_ex(sqlt, cmd_str);
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
