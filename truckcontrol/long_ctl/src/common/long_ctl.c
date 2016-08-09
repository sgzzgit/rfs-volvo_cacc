/* FILE
 *   long_ctl.c
 *
 *   Longitudinal control main program.
 *		Linked with common longititudinal utilities and
 *		particular task functions to form controller programs.
 *
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#include <sys_os.h>
#include "long_ctl.h"
#include <db_utils.h>

/**
 * Timing library calls compute simple stats on system, user and main-loop
 * execution times.
 */
static struct avcs_timing tmg;

/**
 * Signal handling set-up is necessary to allow clean-up operations,
 * like closing the database, to be done when programs is killed
 * with an external signal (e.g., the slay command)
 */

static int sig_list[]=
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        ERROR
};

static jmp_buf exit_env;

static void sig_hand(int sig)
{
        longjmp(exit_env, 1);
}

/** Used to control whether printing occurs in the tasks
 */
int long_ctl_verbose = 0;	// set to 1 with flag -v

/**
 * Main program calls common functions in long_utils.c and functions
 * init_tasks and run_tasks that are different for each controller 
 */
int main(int argc, char *argv[])
{
        posix_timer_typ *ptimer;        /* Timing proxy */
        db_clt_typ *pclt;               /* Pointer to database */
        long_ctrl control;              /* Control structure */
        long_output_typ cmd_var;        /* Command variable to database */
        char hostname[MAXHOSTNAMELEN+1];
	char *progname = argv[0];/* argument vector starts with program name */ 	
	char fname[80];		/* test name */
	int exit_code;		/* value returned from signal handler */
        int max_iterations;	/* usually set to 0, to indicate no limit,
				 * but for some tests we may want a limit */
	FILE *fstream;

	memset(&control, 0, sizeof(long_ctrl) );
        /* Initialize parameters in the control structure */
        if (!long_set_params(argc, argv, &control.params)){
                fprintf(stderr, "Parameter initialization err, %s\n", progname); 
                exit(EXIT_FAILURE);
        }
	max_iterations = control.params.max_iterations;

        /* Initialize the database. */
        get_local_name(hostname, MAXHOSTNAMELEN);

        if((pclt = long_database_init(hostname, progname, &control)) == NULL)
        {
                fprintf(stderr, "Database init error, %s hostname %s\n", progname, hostname);
                exit(EXIT_FAILURE);
        }

	/* construct file name for timing file */
	sprintf(fname, "%s%d.tmg", control.params.data_file, control.params.cmd_test);

        /* On signal or error, execute this code to logout of database */
        if ((exit_code=setjmp(exit_env)) != 0) {
		if (!exit_tasks(pclt, &control, &cmd_var)) 
			fprintf(stderr, "exit_tasks returned error\n");
                
		/* Save command to the database, where actuators will read */
                if(!clt_update(pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE,
                    sizeof(long_output_typ), (void *) &cmd_var))
                        fprintf(stderr, "long_ctl: long_output_var not updated on exit\n"); 
         
	        clt_logout(pclt);       
                avcs_end_timing(&tmg);
		
		printf("%s\n", fname);
		fflush(stdout);
		fstream = fopen(fname, "w+");
                avcs_print_timing(fstream, &tmg);
		fclose(fstream);
		printf("%s exiting, %d exit code\n", argv[0], exit_code); 
                exit(EXIT_SUCCESS);
        }
        else
                sig_ign(sig_list, sig_hand);

/** Define NO_CAN_MESSAGES if you want to test simple process operation when no
 *  CAN messages are arriving. Otherwise initialization will not continue
 *  until crucial messages have been received from the Jbus.
 */
#ifdef NO_CAN_MESSAGES
       /* Wait until any conditions necessary before starting control are met */
        if (!long_wait_for_init(pclt, &control)){
                fprintf(stderr, "Error waiting for init, %s\n", progname);
                longjmp(exit_env,1);
        }
#endif

	/* Initialization specific to each controller */
	if (!init_tasks(pclt, &control, &cmd_var)) {
		fprintf(stderr, "Task initialization failed for %s\n", progname);
		longjmp(exit_env, 3);
	}

	printf("control.params.ctrl_interval %d\n", control.params.ctrl_interval);
        /* Initialize timer used in main loop */
        if ((ptimer = timer_init(control.params.ctrl_interval, 0)) == NULL) {
                fprintf(stderr, "Unable to initialize %s timer\n", progname);
                longjmp(exit_env,2);
        }

        avcs_start_timing(&tmg);	
sleep(1);

        /* Main control loop */
        for (; ;) {
                tmg.exec_num++;		/* used by timing library */

		/* read vehicle information */
		if (!long_read_vehicle_state(pclt, &control))
			fprintf(stderr, "long_read_vehicle_state failed\n");

		/* call the main controller routine */
                if (run_tasks(pclt, &control, &cmd_var) != 0)
                        break;

                /* save command to the database, where actuators will read */
                if(!clt_update(pclt, DB_LONG_OUTPUT_VAR, DB_LONG_OUTPUT_TYPE,
                    sizeof(long_output_typ), (void *) &cmd_var))
                        break;
                
		/* exit if a maximum iteration count is exceeded */
                if (max_iterations && tmg.exec_num > max_iterations )
                        break;

                /* wait for proxy from timer */
                TIMER_WAIT(ptimer);
        }
        longjmp(exit_env,4);    /* error condition in loop */
}
