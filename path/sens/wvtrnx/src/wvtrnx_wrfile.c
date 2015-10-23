/**\file wvtrnx_wrfile.c
 *
 * Copyright (c) 2009   Regents of the University of California
 *
 *  Process to read Wavetronix messages, parse that information, and put 
 *  the value into the data server.
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.  Upon termination log out of the database.
 *
 */

#include <sys_os.h>	/// In path/local, OS-dependent definitions
#include <sys_rt.h>	/// In path/local, "real-time" definitions
#include <sys_ini.h>	/// In path/local, for reading configuration files
#include <timestamp.h>	/// In path/local, for using hh:mm:ss.sss timestamps
#include "db_clt.h"	/// In path/db, for using basic DB client services
#include "db_utils.h"	/// In path/db, for using DB client convenience utilities
#include "wvtrnx.h"	/// Device-specific header file


/** Signal handling required set-up
 */
static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

jmp_buf env;
int status;
static void sig_hand(int sig);
static void debug_only( unsigned char readbuff, int index);

int main(int argc, char *argv[])
{
	unsigned char readbuff[MAX_WVTRNX_MSG_LENGTH+10];
	char hostname[MAXHOSTNAMELEN+1];
	int option;
	char *domain = DEFAULT_SERVICE; /// set up in sys_os.h, used for DB data server 
	int xport = COMM_OS_XPORT;	/// set up in sys_os.h, used for DB data server
	timestamp_t ts;			/// use when writing to dataserver
	db_clt_typ *pclt = NULL;               /// Database client pointer
	int verbose = 0;		/// Set to 1 to print debug
	int msg_count = 0;	/// Counts messages from Wavetronix

	int index = 0;
	ssize_t val;
	int track_no = 0;
	unsigned int checksum = 0;

	posix_timer_typ *ptimer;
	int interval = 100;

        /** Get command line options */
	while ((option = getopt(argc, argv, "i:v")) != EOF) {
		switch(option) {
		case 'i':
			interval = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			printf("Usage: %s [-i<millisec.> -v]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/* Log in to the database (shared global memory).  Default to the
	* the current host. */
	get_local_name(hostname,MAXHOSTNAMELEN);
	if ((pclt = db_list_init(argv[0], hostname, domain, xport,
	      NULL, 0, NULL, 0)) == NULL) {
		printf("Database initialization error in %s. \n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/** Initialize delay timer
	 *
	 */
	ptimer = timer_init(interval, 0);

	/** Here is where program will go when it handles a signal
	 */
	if (setjmp(env) != 0) {
		if (pclt != NULL)
			db_list_done(pclt, NULL, 0, NULL, 0);
		fprintf(stderr, "\nWavetronix exits, %d messages received\n", msg_count);
		fflush(stderr);
		exit(EXIT_SUCCESS);
	} else 
		    sig_ign(sig_list, sig_hand);

	while(1) {
		msg_count++;

		wvtrnx_track_t wvtrnx_track;
		wvtrnx_msg_t wvtrnx_msg;

		get_current_timestamp(&ts);
		if (verbose) {
			print_timestamp(stdout, &ts);
			printf(" ");
			fflush(stdout);
		}
	
		if (pclt != NULL)
			db_clt_read(pclt, DB_WVTRNX_RADAR0_VAR, sizeof(wvtrnx_msg_t), &wvtrnx_msg);
		else
			fprintf(stderr, "NULL data server\n");

		print_wvtrnx(wvtrnx_msg);
		TIMER_WAIT(ptimer);
	}
}

static void sig_hand(int sig)
{
	if(sig == SIGALRM)
		return;
	else
		longjmp(env, sig);
}

static void debug_only( unsigned char readbuff, int index)
{
	printf("(%03d 0x%02hhx) ", readbuff, readbuff);

	if(index == 84)
		printf("\n");

	fflush(stdout);

}
