/* \file 
 *
 * 	Copyright (c) 2007   Regents of the University of California
 *
 *	When the "db" data server is used with only a small number
 *	of interacting processes, it is convenient for the write
 *	processes to create the variables in the data server.
 *	However, when many processes are interacting, and it is
 *	desirable to test different subsets of them for partial
 *	functionality, it is more convenient for one process to
 *	create all the "db" variables that represent shared data
 *	between the processes.
 *
 *	This process should create ALL the variables needed for WAVETRONIX debugging.
 */

#include <sys_os.h>
#include <signal.h>
#include <setjmp.h>
#include <local.h>
#include <sys_rt.h>
#include <sys_list.h>
#include <db_clt.h>
#include "db_utils.h"
#include "timestamp.h"
#include "wvtrnx.h"

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(exit_env, code);
}

// All variables used in messages are created
static db_id_t db_vars_list[] = {
        {DB_WVTRNX_RADAR0_VAR, sizeof(wvtrnx_msg_t)},
};

#define NUM_DB_VARS	sizeof(db_vars_list)/sizeof(db_id_t)

int main(int argc, char *argv[])
{
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	int option;
	char *domain = DEFAULT_SERVICE;
	int xport = COMM_PSX_XPORT;
	char dummy[MAX_DATA_SIZE];
	posix_timer_typ *ptimer;
	int delay_ms = 1000;	// wake up once a second
	int i;

        while ((option = getopt(argc, argv, "d:")) != EOF) {
                switch(option) {
		case 'd':
			domain = strdup(optarg);
			break;
		default:
			fprintf(stderr, "option not recognized\n");
			exit(1);
			break;
                }
	}

	/* Log in to the database (shared global memory).  Default to the
	 * the current host. */
	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = db_list_init(argv[0], hostname, domain, xport, 
			db_vars_list, NUM_DB_VARS,
			NULL, 0)) == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if (setjmp(exit_env) != 0) {

		/* Log out from the database. */
		if (pclt != NULL)
			db_list_done(pclt, db_vars_list, NUM_DB_VARS, NULL, 0);

		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

        if ((ptimer = timer_init( delay_ms, ChannelCreate(0) )) == NULL)
        {
                fprintf(stderr, "Unable to initialize delay timer\n");
                exit(EXIT_FAILURE);
        }

	// Write 0s to all data bucket variables, to initialize
	memset(dummy, 0, MAX_DATA_SIZE);
        for (i = 0; i < NUM_DB_VARS; i++) {
                int id = db_vars_list[i].id;
		int size = db_vars_list[i].size;
		db_clt_write(pclt, id, size, (void *) dummy); 
        }


	// Loop forever doing nothing until terminated with signal
	while (TRUE)
		TIMER_WAIT (ptimer);
}

