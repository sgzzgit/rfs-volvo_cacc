/**\file 
 *
 * ptsnd.c     Reads the most recent GPS point from an sqlite
 *	       table and sends as a UDP message. 
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

/// Query command sent to database
// char *cmd_str = "select utc_time, utc_ms, latitude, longitude, speed, heading, pos, num_sats, hdop, sequence_no from pttbl order by date_time desc limit 1";

char cmd_str[] = "select * from pttbl limit 1";

static path_gps_point_t gps_pt;	/// can't seem to pass data to callback
static int verbose = 0; 	/// if 1, print extra info for debugging

int get_blob_from_string(char *blob_str, unsigned char *blob, int length)
{
	int i;
	char hex_str[5];	/// "0x", two hex digits and final '\0';
	hex_str[0] = '0';
	hex_str[1] = 'x';
	hex_str[4] = '\0';
	if (blob_str[0] != 'x' && blob_str[0] != 'X') {
		return 0;
	}
	for (i = 0; i < length; i++) {
		hex_str[2] = blob_str[2*i];
		hex_str[3] = blob_str[2*i+1];
		sscanf(hex_str, "%hhi", &blob[i]);
	} 
	return 1;
}
/**
 *      callback function, used with query returning statements
 *      to handle results
 */
static int callback(void *not_used, int argc, char **argv, char **az_col)
{
        path_gps_point_t *pp = &gps_pt; 
	char tsbuf[20];
	char msbuf[5];

        if (verbose) {
                int i;
                printf("fill_from_row: %d\n", argc);
                for (i = 0; i < argc; i++) {
                        printf("%s %s\n", az_col[i], argv[i]);
                }
                fflush(stdout);
        }
	if ((strlen(argv[0]) != 15) || !get_blob_from_string(argv[0],
					 &pp->object_id[0], GPS_OBJECT_ID_SIZE))
		printf("Badly formatted blob string %s\n", argv[0]); 

	strncpy(tsbuf, argv[1], 20);		// should always be HH:MM:SS
	snprintf(msbuf, 5, ".%03d", argv[2]);	// arg[1] should always be ms
        strncat(tsbuf, msbuf, 20);		// put in PATH timestamp format
	str2timestamp(tsbuf, &pp->local_time);	
	if (verbose) {
		printf("%s\n", tsbuf);
		print_timestamp(stdout, &pp->local_time);
	}

	strncpy(tsbuf, argv[3], 20);		// should always be HH:MM:SS
	snprintf(msbuf, 5, ".%03d", argv[4]);	// arg[1] should always be ms
        strncat(tsbuf, msbuf, 20);		// put in PATH timestamp format
	str2timestamp(tsbuf, &pp->utc_time);	

        pp->latitude = atof(argv[5]);
        pp->longitude = atof(argv[6]);
        pp->speed = atof(argv[7]);
        pp->heading = atof(argv[8]);
        pp->pos = atoi(argv[9]);
        pp->num_sats = atoi(argv[10]);
        pp->hdop = atof(argv[11]);
        pp->sequence_no = atof(argv[12]);
	return 0;
}

int main(int argc, char *argv[])
{
	int do_timing = 0;	/// if 1, print user and system time 
	struct avcs_timing timing;	/// used to log timing info
	int option;		/// used with getopt

	char *udp_name = "192.168.255.255";	/// default broadcast 
	int udp_port = 7015;
        int sd_out;             /// socket descriptor for UDP send
        int bytes_sent;         /// returned from sendto
        struct sockaddr_in snd_addr;    /// used in sendto call
	int do_broadcast = 1;	/// by default, broadcast socket

	sqlite3 *sqlt;
	char *sqlt_name = "my_gps.db";/// default name for sqlite database
	char cmd_str[MAX_CMD_STR];	/// pass to sqlite3_exec
	char *zerr = NULL;
	path_gps_point_t pt;		/// returned from query 
	int last_sequence_no;		/// don't send duplicates 
	int sqlite_err = 0;		/// keep count of locking errors

 	/** to avoid conflicts with 1Hz GPS inserts, choose timer interval
	 * relatively prime to 1000
	 */
	int timer_interval = 1000;       
        posix_timer_typ *ptmr;          // to wait between sends

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "a:d:i:ntu:v")) != EOF) {
		switch(option) {
                case 'a':
                        udp_name = strdup(optarg);
                        break;
		case 'd':
			sqlt_name = strdup(optarg);
			break;
		case 'i':
			timer_interval = atoi(optarg);
			break;
                case 'n':
                        do_broadcast = 0;
                        break;
		case 't':
			do_timing = 1;	/// prints sys and user time
			break;
                case 'u':
                        udp_port = atoi(optarg);
                        break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
                        fprintf(stderr, " -a <dest IP (default broadcast)>\n");
			fprintf(stderr, " -d  <database name>\n ");
			fprintf(stderr, " -i  <send interval, ms>\n ");
			fprintf(stderr, " -n  (unicast, broadcast default)\n ");
			fprintf(stderr, " -t  (measure timing) \n ");
                        fprintf(stderr, " -u  (UDP port number for output) ");
			fprintf(stderr, " -v  (verbose mode) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
        if (do_broadcast) {
                sd_out = udp_broadcast();
        } else
                sd_out = udp_unicast();

        if (sd_out < 0) {
                printf("failure opening socket on %s %p\n",
                                 udp_name, udp_port);
        }

	if (do_timing)
		avcs_start_timing(&timing);

	if (sqlite3_open(sqlt_name, &sqlt) != 0) {
		fprintf(stderr, "Can't open %s\n", sqlite3_errmsg(sqlt));
		sqlite3_close(sqlt);
		exit(EXIT_FAILURE);
	}
        if ((ptmr = timer_init(timer_interval, 0)) == NULL) {
               fprintf(stderr, "timer_init failed\n");
               exit(EXIT_FAILURE);
        }
	printf("timer_interval %d\n", timer_interval);
		
	if (setjmp(exit_env) != 0) {
		if (do_timing) {
			printf("Exiting %s\n", argv[0]);
			fflush(stdout);
			avcs_end_timing(&timing);
			avcs_print_timing(stdout, &timing);
			fflush(stdout);
		}
		sqlite3_close(sqlt);
		printf("Sqlite3 err count %d\n", sqlite_err);
		fflush(stdout);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	memset(&pt, 0, sizeof(pt));

	last_sequence_no = 0;	/// valid numbers start at 1
	while (1) {
		timing.exec_num++;
		if (sqlite3_exec(sqlt,
			 "select * from pttbl order by utc_time desc limit 1",
				 callback, 0,
				 &zerr) != SQLITE_OK) {
			if (verbose)
				fprintf(stderr, "sqlite3 error: %s\n", zerr);
			else
				sqlite_err++;
			sqlite3_free(zerr);
		}  
		if (verbose) {
			path_gps_print_point(stdout, &gps_pt);
			printf("\n");
		}

		/// send only on update; may want to change this
		/// to give quicker response to listeners coming in range

                set_inet_addr(&snd_addr, inet_addr(udp_name), udp_port);
                if  (gps_pt.sequence_no > last_sequence_no) {
                        bytes_sent = sendto(sd_out, &gps_pt, sizeof(gps_pt), 0,
                                        (struct sockaddr *) &snd_addr,
                                        sizeof(snd_addr));
			last_sequence_no = gps_pt.sequence_no;
			/// on wraparound, make sure it is non-negative
			/// and less that the least valid sequence number
			if (last_sequence_no + 1 <= 0)
				last_sequence_no = 0;
                }
                TIMER_WAIT(ptmr);
	}
}

