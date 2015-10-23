/**\file 
 *
 * coswmsgrcv.c     
 *	 	Read Curve Over Speed Warning map data. Write
 *		data to a file and record to an sqlite3 database.
 *
 *		Records local timestamp when received.
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
#include <cosw.h>

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static char *cosw_str = "mapupdate (bytecount, date_time)";

void coswmap_insert_str(char *cmd_str, int bytecount) 
{
        char tmp_str[MAX_CMD_STR];
        snprintf(tmp_str, MAX_CMD_STR, "(%d, %s)",
			bytecount, "datetime('now')"); 
        snprintf(cmd_str, MAX_CMD_STR, "insert into %s values %s",
                 cosw_str, tmp_str);
}

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
	int udp_port = 3335;	/// port for receiving curve over speed map
	int option;

	char buf[1024];		/// buffer to hold map fragment
	int sd_in;		/// socket descriptor for UDP receive
	int bytes_rcvd;		/// returned from recvfrom
	struct sockaddr_in src_addr;	/// used in recvfrom call
	unsigned int socklen;
        sqlite3 *sqlt;
        char *sqlt_name = "cosw.db";	/// default name for sqlite database
	char *out_basename = "coswmap";	/// default output string
        char foutname[20];	/// hold output name (file per message) 
	int fdout;
	char cmd_str[MAX_CMD_STR];

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "d:vtu:")) != EOF) {
		switch(option) {
		case 'd':
			sqlt_name = strdup(optarg);	
			break;
		case 'o':
			out_basename = strdup(optarg);	
			break;
		case 't':
			do_timing = 1;	// call timing utilities	
			break;
		case 'u':
			udp_port = atoi(optarg);
			break;
		case 'v':
			verbose = 1;	//
			break;
		default:
			fprintf(stderr, "Usage %s: ", argv[0]); 
			fprintf(stderr, " -d <database name> ");
			fprintf(stderr, " -t (do timing)");
			fprintf(stderr, " -u  (UDP port number for input) ");
			fprintf(stderr, " -v  (verbose) ");
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
		bytes_rcvd = recvfrom(sd_in, buf, 1024, 0,
				(struct sockaddr *) &src_addr, &socklen);
		if (bytes_rcvd < 0) {
			perror("recvfrom failed\n");
			continue;
		}
		snprintf(foutname, 20, "%s%d.out", out_basename, timing.exec_num); 
		fdout = open(foutname, O_WRONLY | O_CREAT);
		fprintf(stderr, "Output: %s\n", foutname);

		if (verbose) {
			printf("%d %d\n",timing.exec_num, bytes_rcvd);	
			fprintf(stdout, "\n");
		}
		write(fdout, buf, bytes_rcvd);
		close(fdout);

		coswmap_insert_str(cmd_str, bytes_rcvd); 
		if (verbose)
			printf("curve OSW map insert string: %s\n", cmd_str);

		sqlt3_ex(sqlt, cmd_str);
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
