/**\file 
 *
 * bcnrcv.c     Receives VIICA beacon, reads lat/long from data server,
 *		and writes combined record to file, for range testing.
 *
 *		Records local timestamp when received as well as 
 *		time from message.	
 *	
 *  Copyright (c) 2008   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <sys_list.h>
#include <sys_buff.h>
#include <local.h>
#include <timing.h>
#include <timestamp.h>
#include <udp_utils.h>
#include <data_log.h>
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

/// May need to fix this depending on order of bytes.
/// 5 bytes represent milliseconds since 1/1/2004
double five_bytes_to_sec(unsigned char *viica_time)
{
	int i;
	double result = 0;
	for (i = 0; i < 5; i++) {
		result *= 256.0;
		result = result + viica_time[i];
	}
	return (result /= 1000.0);	/// convert milliseconds to seconds
}

int main(int argc, char *argv[])
{
	int status;
        char hostname[MAXHOSTNAMELEN];
	char *domain = DEFAULT_SERVICE;
        gps_data_typ gps_data;
        timestamp_t ts;
        db_clt_typ *pclt;       /// data server client pointer
        int xport = COMM_PSX_XPORT;
	
        char sentence[MAX_GPS_SENTENCE];
	FILE *fpout;		/// file pointer to write output 
	int old_fileday = 99;	/// invalid day value for initialiation
	int file_serialno = 0;	/// start numbering files at 0
	char suffix[80];	/// file extension for data logging 
	char prefix[80];	/// identifier for files logged 
	double start_time;	/// when last file was opened
	int file_time = 5;	/// minutes file duration 
	
	int verbose = 0;	/// print extra info for debugging	
	int udp_port = 6050;	/// port for receiving beacon 
	int do_timing = 0;	/// if 1, print user and system time
	struct avcs_timing timing;
	int option;

	unsigned char buf[512]; /// fill in from beacon received
	int sd_in;		/// socket descriptor for UDP send
	int bytes_rcvd;		/// returned from recvfrom
	struct sockaddr_in src_addr;	/// used in recvfrom call
	unsigned int socklen;

	unsigned int rsu_id;	/// 4 bytes at byte 117

	/// milliseconds past 1/1/2004 currently in GMT
	double beacon_time;	/// in seconds

	strncpy(suffix, ".dat", 80);
	strncpy(prefix, "/big/data", 80);
	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "f:vu:t")) != EOF) {
		switch(option) {
		case 'f':
			strncpy(prefix, optarg, 80);
			break;
		case 't':
			do_timing = 1;
			break;
		case 'v':
			verbose = 1;
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
	printf("PATH GPS receive app, files output to %s\n", prefix);
	fflush(stdout);

	sd_in = udp_allow_all(udp_port);
	if (sd_in < 0) {
		printf("failure opening socket on %d\n", udp_port);
		exit(EXIT_FAILURE);
	}

	if (!open_data_log(&fpout, prefix, suffix, &start_time,
				 &old_fileday, &file_serialno, NULL)){
			 printf("Error opening %s%d%s\n",
				 prefix, file_serialno, suffix);
			 exit (EXIT_FAILURE);
	}
	get_local_name(hostname, MAXHOSTNAMELEN);
	if (( pclt = db_list_init( argv[0], hostname, domain, xport,
		NULL, 0, NULL, 0)) == NULL ) {
		printf("Database initialization error in %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (do_timing)
		avcs_start_timing(&timing);

	if (setjmp(env) != 0) {
		if (do_timing) {
			printf("Exiting bcnrcv\n");
			fflush(stdout);
			avcs_end_timing(&timing);
			avcs_print_timing(stdout, &timing);
			fflush(stdout);
		}
		db_list_done(pclt, NULL, 0, NULL, 0);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	socklen = sizeof(src_addr);
	memset(&src_addr, 0, socklen);

	while (1) {
		timestamp_t ts;
		bytes_rcvd = recvfrom(sd_in, buf, 512, 0,
				(struct sockaddr *) &src_addr, &socklen);
		if (bytes_rcvd < 0) {
			perror("recvfrom failed\n");
			continue;
		}
		db_clt_read(pclt, DB_GPS_RMC_VAR, sizeof(gps_data), &gps_data);
		
		get_current_timestamp(&ts);
		print_timestamp(fpout, &ts);

		/// use source IP to identify source of message in output
		/// currently same for all RSUs but this may change
		fprintf(fpout, " 0x%08x ", src_addr.sin_addr.s_addr);

		rsu_id = ntohl(*((unsigned int *) &buf[117]));	
		beacon_time = five_bytes_to_sec(&buf[35]);
		fprintf(fpout, "%d %.3f %.6f %.6f", rsu_id,
				beacon_time, 
				path_gps2degree(gps_data.data.rmc.latitude), 
				path_gps2degree(gps_data.data.rmc.longitude));
		fprintf(fpout, "\n");

		if (verbose) {
			fprintf(stdout, " 0x%08x ", src_addr.sin_addr.s_addr);
			fprintf(stdout, "\n");
		}

		reopen_data_log(&fpout, file_time,
                                        prefix, suffix, 
                                        &start_time, &old_fileday,
                                        &file_serialno, NULL, NULL);
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
