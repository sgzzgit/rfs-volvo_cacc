/* FILE
 *  sndprof.c
 *              Updates longitudinal output variable according to a profile
 *		in a file made by vehprof. Options allow either engine speed
 *		or torque to be used. 
 *
 *	Usage: sndprof [-f input file], 
 *		  -v (enable speed mode) -t (enable torque)
 *		  [-s number of speed commands before alternate to torque)
 *		  [-q number of torque commands before alternate to speed)
 *
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

/* Longitudinal control variable changes every 20 milliseconds */
#define INTERVAL_MSECS	20

#include "profile.h"
#include "vehicle_profile.h"

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

/* assumes we are alternating between speed and torque control modes */

static int set_engine_mode (int *cmd_count, int engine_mode,
			 int speed_count, int torque_count)

{
	int count = *cmd_count;
	int new_mode;
	if (count == 0 && engine_mode == TSC_SPEED_CONTROL) {
		count = torque_count;
		new_mode = TSC_TORQUE_CONTROL;
	} else if (count == 0 && engine_mode == TSC_TORQUE_CONTROL) {
		count = speed_count;
		new_mode = TSC_SPEED_CONTROL;
	} else {	 /* count is greater than 0 */
		count--;
		new_mode = engine_mode;
	}
	*cmd_count = count;
	return (new_mode);
}
	
int main( int argc, char *argv[] )
{
	int ch;		
	posix_timer_typ *ptimer;       /* Timing proxy */
	int interval = INTERVAL_MSECS;
	int i = 0;
	long_output_typ cmd_var;
	int engine_mode = TSC_SPEED_CONTROL; /* or TSC_TORQUE_CONTROL */
	int debug=0;
	int alternate = 0; /* if >0, alternate speed and torque commands */
	int alternate_speed=-1; /* set to count for run of speed commands */
	int alternate_torque=-1; /* set to count for run of torque commands */
	int cmd_count;	/* used with alternating speed/torque commands */
	char *filename = "trq.dat";
	profile_item *profile_array;
	int num_commands = 0;	/* set to number of commands in profile */
	struct timeb reference_time;
	struct timeb current_time;
	struct timeb last_spdctl_disable;
	int spdctl_time_limit = 4000;	/* milliseconds */
	db_clt_typ * pclt;
	FILE *snd_stream;

        while ((ch = getopt(argc, argv, "df:i:vta:s:q:")) != EOF) {
                switch (ch) {
		case 'd': debug = 1;
			  break;
		case 'f': filename = optarg;
			  break;
		case 'i': interval = atoi(optarg);
			  break;
		case 'v': engine_mode = TSC_SPEED_CONTROL;
			  break;
		case 't': engine_mode = TSC_TORQUE_CONTROL;
			  break;
		case 's': alternate_speed = atoi(optarg);
			  break;
		case 'q': alternate_torque = atoi(optarg);
			  break; 
		default:  printf("Usage: %s [-f input file]\n", argv[0]);
			  printf("\t-v (enable speed mode) -t (enable torque)\n");
			  printf("\t[-s number of speed commands before alternate to torque\n");
			  printf("\t[-q number of torque commands before alternate to speed\n");

			  exit(EXIT_FAILURE);
                          break;
                }
        }

	/* If only one of alternate_speed and alternate_torque is specified,
	 * set the one which is undefined to the one already defined
	 * If neither is specified, do not alternate, use speed control
	 * mode if neither -v nor -t was specified.
	 */
	alternate = alternate_speed > 0 || alternate_torque > 0;

	if (alternate_speed > 0 && alternate_torque == -1) 
		alternate_torque = alternate_speed;
	else if (alternate_speed == -1  && alternate_torque > 0) 
		alternate_speed = alternate_torque;
	
	/* Initialize input array */
	profile_array = read_profile(filename, &num_commands);

	printf("Opened file %s, num_commands %d, profile_array 0x%x\n",
			filename, num_commands, (unsigned int) profile_array);
	fflush(stdout);

	/* Initialize the database. */
	if ((pclt = database_init_for_profile(argv, 1 )) == NULL) {
		printf("Database initialization error in sndprof\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	if (clt_create(pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE,
		    sizeof( long_output_typ )) == FALSE ) {
		if (debug)
		    printf("Longitudinal output variable already created?\n");
	}

	/* Initialize the timer */
	if ((ptimer = timer_init( interval,0 )) == NULL) {
		printf("Unable to initialize sndprof timer\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	if( setjmp( exit_env ) != 0 ) {
		/* Set override disable before exiting */
		cmd_var.engine_retarder_command_mode = TSC_OVERRIDE_DISABLED;
		cmd_var.engine_command_mode = TSC_OVERRIDE_DISABLED;
		cmd_var.brake_command_mode = XBR_NOT_ACTIVE;

		if( clt_update( pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE,
		    sizeof( long_output_typ ), (void *) &cmd_var ) == FALSE )
		    printf("sndprof: clt_update error\n", DB_LONG_OUTPUT_VAR);
		clt_logout( pclt );	
		avcs_end_timing(&tmg);
		snd_stream = fopen("sndprof.tmg", "w");
		avcs_print_timing(snd_stream, &tmg);
		fclose(snd_stream);
		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	avcs_start_timing(&tmg);

	ftime(&reference_time);
	last_spdctl_disable = reference_time;

	if (alternate) {
		cmd_count = alternate_speed;
		engine_mode = TSC_SPEED_CONTROL;
	}

	fflush(stdout);

	for (i = 0 ;i < num_commands ;) {
		double time_in_seconds;
		

		tmg.exec_num++;
	   	
		ftime(&current_time);
		time_in_seconds =
			TIMEB_SUBTRACT(&reference_time, &current_time)/1000.0;

		if ((engine_mode == TSC_SPEED_CONTROL) && !alternate &&
			TIMEB_SUBTRACT(&last_spdctl_disable, &current_time)
				> spdctl_time_limit) {
			if (debug) {
				printf("Sending override disable\n");
				fflush(stdout);
			}
			cmd_var.engine_command_mode = TSC_OVERRIDE_DISABLED;
			if( clt_update( pclt, DB_LONG_OUTPUT_VAR,
					DB_LONG_OUTPUT_TYPE,
					sizeof(long_output_typ),
					 (void *) &cmd_var ) == FALSE )
				printf("clt_update error, var %d\n",
					DB_LONG_OUTPUT_VAR);
			cmd_var.engine_command_mode = TSC_SPEED_CONTROL;
			last_spdctl_disable = current_time;
		} else if (time_in_seconds >= profile_array[i].time) {
			if (alternate) {
				engine_mode = set_engine_mode(&cmd_count,
						 engine_mode,
						 alternate_speed,
						 alternate_torque);
			}
			copy_to_command(&profile_array[i], &cmd_var,
					 engine_mode);
			if (debug) {
				printf("send command %d, mode %d\n", i,
					cmd_var.engine_command_mode);
				fflush(stdout);
			}

			/* Now save this command to the database. */
			if( clt_update( pclt, DB_LONG_OUTPUT_VAR,
					DB_LONG_OUTPUT_TYPE,
					sizeof(long_output_typ),
					 (void *) &cmd_var ) == FALSE )
				printf("clt_update error, var %d\n",
					DB_LONG_OUTPUT_VAR);

			i++;
		}

		/* Now wait for proxy from timer */
		TIMER_WAIT(ptimer);
	}
	fprintf(stderr, "Finished sending %d commands\n", num_commands);
	longjmp(exit_env,2);	/* go to exit code when loop terminates */
}
