/* \file 
 *
 * 	Copyright (c) 2009   Regents of the University of California
 *
 *	This process calls long_database_init in ../common/long_utils.c
 *	to create all the input data server variables needed by the long_trk
 *	process. It then calls create on the output variables needed
 *	by the long_ctl process.
 */

#include <sys_os.h>
#include <timestamp.h>
#include <path_gps_lib.h>
#include <sys_rt.h>
#include <sys_list.h>
#include <db_clt.h>
#include <clt_vars.h>	
#include <db_utils.h>
#include <jbus_vars.h>
#include <path_gps_lib.h>
#include <long_ctl.h>
#include <evt300.h>
#include <mdl.h>
#include <densolidar.h>
#include <long_comm.h>
#include <dvi.h>
#include <veh_trk.h>
#include "jbussendGPS.h"

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

/** Output variables for the long_trk process are created with this list. 
 */
static db_id_t db_vars_list[] = {
	{DB_LONG_OUTPUT_VAR, sizeof( long_output_typ )},
	{DB_LONG_DIG_OUT_VAR, sizeof( long_dig_out_typ )},
	{DB_COMM_TX_VAR, sizeof( veh_comm_packet_t )},
	{DB_MDL_LIDAR_VAR, sizeof( mdl_lidar_typ )},
        {DB_ENGINE_DEBUG_VAR, sizeof(can_debug_t)},
        {DB_ENGINE_RETARDER_DEBUG_VAR, sizeof(can_debug_t)},
        {DB_DVI_RCV_VAR, sizeof(char)},
        {DB_DVI_OUT_VAR, sizeof(dvi_out_t)},
};

#define NUM_DB_VARS	sizeof(db_vars_list)/sizeof(db_id_t)

int main(int argc, char *argv[])
{
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	int option;
	long_ctrl control_state;
	posix_timer_typ *ptimer;
	int delay_ms = 1000;	// wake up once a second
	char dummy[MAX_DATA_SIZE];	// used when initializing variables
	int i;
	int verbose = 0;
	timestamp_t current_ts;

        while ((option = getopt(argc, argv, "t:v")) != EOF) {
                switch(option) {
		case 't':
			delay_ms = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s -t <millisec> -v (verbose)\n",				argv[0]);
			exit(1);
			break;
                }
	}

	get_local_name(hostname, MAXHOSTNAMELEN);

	/** Call long_database_init to login in to database, initialize 
	 * jbus variables and otherinput variables to long_ctl process 
	 * Initialize control structure first to set vehicle type
	 */

	memset(&control_state, 0, sizeof(control_state));
	ftime(&control_state.start_time);
	control_state.params.vehicle_type = VEH_TYPE_TRUCK_SILVR;

	if ((pclt = long_database_init(hostname, argv[0], &control_state)) 
			 == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}


	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if (setjmp(exit_env) != 0) {
		if (pclt != NULL)
			clt_logout(pclt); // clt_destroy not necessary
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

        if ((ptimer = timer_init( delay_ms, ChannelCreate(0) )) == NULL)
        {
                fprintf(stderr, "Unable to initialize delay timer\n");
                exit(EXIT_FAILURE);
        }

	// Create and initialize additional variables  to 0
        for (i = 0; i < NUM_DB_VARS; i++) {
                int id = db_vars_list[i].id;
		int size = db_vars_list[i].size;
		clt_create(pclt, id, id, size);
        }
	memset(dummy, 0, MAX_DATA_SIZE);

        for (i = 0; i < NUM_DB_VARS; i++) {
                int id = db_vars_list[i].id;
		int size = db_vars_list[i].size;
		db_clt_write(pclt, id, size, (void *) dummy); 
        }

	// Loop forever doing nothing until terminated with signal
	// In verbose mode print timestamp to track that process is alive 
	while (TRUE) {
		if (verbose) {
                       get_current_timestamp(&current_ts);
                       print_timestamp(stdout,&current_ts);
                       printf("%s \n", argv[0]);
		}
		TIMER_WAIT (ptimer);
	}
}

