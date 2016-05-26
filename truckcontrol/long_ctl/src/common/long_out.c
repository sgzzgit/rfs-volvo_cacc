/* FILE
 * long_out.c   Writes the DB_LONG_OUTPUT variable with values
 *		for a particular command. Loops, waking up
 *		periodically, until killed by signal. Writes
 *		override disable values for engine and retarder
 *		to database variable, then exits. 
 *
 *  Usage: long_out [-v speed(RPM) -t torque(N-m)
 *	  -b deceleration (m/s^2) -n slot -f serial port
 *	  -c state code -d destination -i interval]
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

/* wake-up interval set at 100 milliseconds
 */
#define INTERVAL_MSECS	100

#include "sys_os.h"
#include "local.h"
#include "sys_rt.h"
#include "db_clt.h"
#include "db_utils.h"
#include "clt_vars.h"
#include "veh_trk.h"
#include "timestamp.h"
#include "jbus_vars.h"
#include "jbus_extended.h"
#include "j1939.h"
#include "j1939pdu_extended.h"
#include "j1939db.h"
#include <timing.h>

static struct avcs_timing tmg;

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR
};


static jmp_buf exit_env;


static void sig_hand( int sig)
{
	longjmp( exit_env, 1);
}

int main( int argc, char *argv[] )
{
	int ch;		
        db_clt_typ *pclt;  		/* Database pointer */
	posix_timer_typ *ptimer;      	 /* Timing proxy */
	int interval = INTERVAL_MSECS;
	int engine_mode = TSC_SPEED_CONTROL;
	float rqst_speed = 800.0;		/* RPMs */
	float rqst_torque = 0.0;
	float rqst_brake = 0.0;
	int destination_address = J1939_ADDR_ENGINE;	/* engine; retarder is 15 */

	long_output_typ cmd_var;

        pclt = j1939_database_init(argv);
/*
Destination addresses
#define J1939_ADDR_ENGINE               0
#define J1939_ADDR_TRANS                3
#define J1939_ADDR_BRAKE                11
#define J1939_ADDR_ENG_RTDR             15

Values for the TSC1 (Torque override control mode field (J1939 standard)
define TSC_OVERRIDE_DISABLED   0
define TSC_SPEED_CONTROL       1
define TSC_TORQUE_CONTROL      2
define TSC_SPEED_TORQUE_LIMIT  3

*/

        while ((ch = getopt(argc, argv, "b:d:i:m:t:v:r:")) != EOF) {
                switch (ch) {
		case 'b': rqst_brake = atof(optarg);
			  destination_address = J1939_ADDR_BRAKE;
			  printf("deceleration requested %6.4f\n", rqst_brake);
			  break;
		case 'd': destination_address = atoi(optarg);
			  break;
		case 'i': interval = atoi(optarg);
			  break;
		case 'm': engine_mode = atoi(optarg);
			  printf("mode is %d\n", engine_mode);
			  break;
		case 't': rqst_torque = atof(optarg);
			  destination_address = J1939_ADDR_ENGINE;
			  engine_mode = TSC_TORQUE_CONTROL;
			  printf("engine torque requested %.3f N-m\n", rqst_torque); 
			  break;
		case 'r': rqst_torque = atof(optarg);
			  printf("retarder torque requested %.3f N-m\n", rqst_torque); 
			  destination_address = J1939_ADDR_ENG_RTDR;
			  engine_mode = TSC_TORQUE_CONTROL;
			  break;
		case 'v': rqst_speed = atof(optarg);
			  destination_address = J1939_ADDR_ENGINE;
			  engine_mode = TSC_SPEED_CONTROL;
			  printf("speed requested %.3f RPM\n", rqst_speed); 
			  break;
                default:  printf( "Usage: %s -v engine speed(RPM) -t torque(N-m) -r retarder torque(N-m)", argv[0]);
			  printf("-b deceleration (m/s^2) \n");
                	  printf("-d destination (0-engine, 3=transmission, 11=brake, 15=engine retarder)\n"); 
			  printf("-m mode (1=speed, 2=torque, 3=speed & torque)\n");
			  printf("-i interval\n");
			  exit(1);
                          break;
                }
        }
	fflush(stdout);

	if( setjmp( exit_env ) != 0 ) {
		char tmg_filename[80];
		FILE *tmg_stream;
		cmd_var.engine_command_mode = TSC_OVERRIDE_DISABLED;
		cmd_var.engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		cmd_var.brake_command_mode = XBR_NOT_ACTIVE;
		if( clt_update( pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE,
		    sizeof( long_output_typ ), (void *) &cmd_var ) == FALSE ) {
		    fprintf(stderr, "clt_update( DB_LONG_OUTPUT_VAR )\n" );
		}
		avcs_end_timing(&tmg);
                close_local_database(pclt);
		sprintf(tmg_filename, "tsc_spd_%.3f.tmg", rqst_speed);
		tmg_stream = fopen(tmg_filename, "w");
		avcs_print_timing(tmg_stream, &tmg);
		fclose(tmg_stream);
		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	/** Update database **/
	if (destination_address == J1939_ADDR_ENGINE) { 
		cmd_var.engine_retarder_command_mode = 0;
		
		cmd_var.engine_command_mode = engine_mode;
		cmd_var.brake_command_mode = XBR_NOT_ACTIVE;
		cmd_var.ebs_deceleration = 0.0;
		switch(engine_mode){
			case 0: 	/* disable */
				break;
			case 1: 	/* speed */
				cmd_var.engine_speed = rqst_speed;
				break;
			case 2: 	/* torque */
				cmd_var.engine_torque = rqst_torque;
				break;
			case 3: 	/* speed/torque limit */
				cmd_var.engine_speed = rqst_speed;
				cmd_var.engine_torque = rqst_torque;
				break;
			default:
				fprintf(stderr, "Override mode not recognized for engine in tscj1939\n");
				break;
		}
	}
	else if (destination_address == J1939_ADDR_ENG_RTDR) { 
		cmd_var.engine_command_mode = 0;

		cmd_var.engine_retarder_command_mode = engine_mode;
		cmd_var.brake_command_mode = XBR_NOT_ACTIVE;
		cmd_var.ebs_deceleration = 0.0;
		switch(engine_mode){
			case 0: 	/* disable */
				break;
			case 2: 	/* torque */
			case 3:		/* torque limit */
				cmd_var.engine_retarder_torque = rqst_torque;
				break;
			default:
				fprintf(stderr, "Override mode not recognized for retarder in tscj1939\n");
				break;
		}
	}
	else if (destination_address == J1939_ADDR_BRAKE) {
		/* disable acceleration, don't use engine retarder */
		cmd_var.engine_command_mode = TSC_TORQUE_CONTROL;
		cmd_var.engine_torque = 0.0;
		cmd_var.engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		cmd_var.brake_command_mode = XBR_ACTIVE;
		cmd_var.ebs_deceleration = rqst_brake;
	}
	else {
		fprintf(stderr, "Destination address not recognized\n");
	}

	if( clt_update( pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE,
	    sizeof( long_output_typ ), (void *) &cmd_var ) == FALSE ) {
		    fprintf(stderr, "clt_update( DB_LONG_OUTPUT_VAR )\n" );
	}

	if ((ptimer = timer_init( interval, 0 )) == NULL) {
		printf("Unable to initialize long_out timer\n");
	}

	avcs_start_timing(&tmg);

	for ( ; ; ) {
		tmg.exec_num++;

		/* Loop, on signal will turn off all commands */
		TIMER_WAIT( ptimer );

	}
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}
