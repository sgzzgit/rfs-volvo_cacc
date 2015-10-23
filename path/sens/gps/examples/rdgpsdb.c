/**\file 
 *
 * rdgpsdb.c     Reads a GPS point type from the DB server 
 *		 By default reads DB_GPS_PT_LCL_VAR
 *		 Other value can be specified on the command line
 *		 Prints on stdout.
 *
 *		 By default timer interval is set to 0 and
 *		 only one read is done. Set timer interval
 *		 on the command line to get continuous reads.
 *
 *		 For debugging GPS read and receiving.
 *		 Assumes DB shared variable is created elsewhere.
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
#include <db_clt.h>
#include <db_utils.h>
#include "path_gps_lib.h"
#undef DO_TRACE

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	/// for timer
	ERROR,
};
jmp_buf env;

static void sig_hand( int code )
{
        if (code == SIGALRM)
                return;
        else
                longjmp(env, code);
}

#define MAX_BUFFERED_GPS_DATA 20

int main(int argc, char *argv[])
{
	int status;
	char hostname[MAXHOSTNAMELEN];
	char *domain = DEFAULT_SERVICE;	/// on Linux sets DB q-file directory
        db_clt_typ *pclt;       /// data server client pointer
        int xport = COMM_OS_XPORT;	/// value set correctly in sys_os.h 
	int option;

	path_gps_point_t hb;	/// fill in from GPS messages received
	int num_db = DB_GPS_PT_LCL_VAR;	/// or set from cmd line

	posix_timer_typ *ptmr;
	int interval = 0;

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "n:t:")) != EOF) {
		switch(option) {
		case 'n':
			num_db = atoi(optarg);
			break;
		case 't':
			interval = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -t  <interval between reads> \n ");
			fprintf(stderr, " -n  <use this DB number> \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
	fprintf(stderr, "%s: read GPS points from DB var %d\n", argv[0], num_db);
	fflush(stderr);

	if (interval != 0) {
		if ((ptmr = timer_init(interval, 0)) == NULL) {
			printf("timer_init failed\n");
			exit(EXIT_FAILURE);
		}
	}

	get_local_name(hostname, MAXHOSTNAMELEN);
	if (( pclt = db_list_init( argv[0], hostname, domain, xport,
		NULL, 0, NULL, 0)) == NULL ) {
		printf("Database initialization error in %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	printf("%s: DB initialized\n", argv[0]);
	fflush(stdout);

	if (setjmp(env) != 0) {
		db_list_done(pclt, NULL, 0, NULL, 0);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	memset(&hb, 0, sizeof(hb));

	while (1) {
		db_clt_read(pclt, num_db, sizeof(path_gps_point_t), &hb);
		path_gps_print_point(stdout, &hb);
		printf("\n");
		if (interval == 0)
			longjmp(env, 1);
		else
			TIMER_WAIT(ptmr);
	}
}

