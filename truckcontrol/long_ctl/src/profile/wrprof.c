/* FILE
 *   wrprof.c
 *      	Writes profile files directly during manual driving
 *		or tests of software.
 *	
 *		Different profile headers included and linked with
 *		different profile functions to profile different
 *		data.
 *
 *		Saves data in an array, save to disk at end of run,
 *		if number of items is specified on the command line.
 *	
 *		If number of items is not specified, runs until slain,
 *		writing files to disk whenever array of items fills.
 *		
 *		Flags
 *			-n	size of array
 *			-i	interval between reading variables (ms)
 *		
 * Copyright (c) 2002   Regents of the University of California
 *
 */


#include "profile.h"

#if defined(LONG_OUT_PROFILE)
#include "long_out_profile.h"
#elif defined(VEHICLE_PROFILE)
#include "vehicle_profile.h"
#elif defined(JCMD_PROFILE)
#include "jcmd_profile.h"
#else
error("Must define profile item")
#endif

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR
};


static jmp_buf exit_env;
static int item_count = 0;


static void sig_hand( int sig)
{
	longjmp( exit_env, 1);
}

int main( int argc, char *argv[] )
{
	db_clt_typ *pclt;	/* pointer to CLT database */
	int create_dbvs = 0;	/* if 1, create database variables */
	char *filename = "profile.dat";
	profile_item *profile;
	posix_timer_typ *ptimer;
	struct timeb start_time;
	int interval = 50;	/* default write data every 50 milliseconds */
	int numitems = 2048;	/* default number of items in file */
	int unbounded = 1;	/* write multiple files if needed */
	int ch;		
	int exit_val;		/* return value from setjmp */

	while ((ch = getopt(argc, argv, "cf:i:n:")) != EOF) {
                switch (ch) {
                case 'c': create_dbvs = 1;
                          break;
		case 'f': filename = optarg;
			  break;
                case 'i': interval= atoi(optarg);
                          break;
                case 'n': numitems= atoi(optarg);
			  if (numitems > 0)
				unbounded = 0;
			  else
				numitems = 2048; /* unbounded, default buffer */
                          break;
                default: printf( "Usage: %s \n", argv[0]);
			 printf("\t-c <create database variables> \n");
			 printf("\t-n number of items -i interval (ms) \n");
			 printf("\t-f output filename \n");
			 exit(EXIT_FAILURE);
                         break;
                }
        }

	/* Log in to the database. */
	pclt = database_init_for_profile(argv, create_dbvs);

	/* Allocate the output array */
	printf("numitems %d, sizeof(profile_item %d\n",
		numitems, sizeof(profile_item));
	profile = (profile_item *) malloc(sizeof(profile_item) * numitems);
	printf("Allocated profile array 0x%x, %d items\n", profile, numitems);

	if( (exit_val = setjmp( exit_env )) != 0 ) {
		printf("wrprof: item_count %d, exit code %d\n",
			 item_count, exit_val);
		if (item_count > 0)
			write_profile(filename, item_count, profile);
		clt_logout( pclt );
		exit(EXIT_SUCCESS);
	}
	else
		sig_ign( sig_list, sig_hand );

	/* Setup a timer for every 'interval' msec. */
	if ((ptimer = timer_init( interval, 0 )) == NULL) {
		printf("Unable to initialize %s timer\n", argv[0]);
		longjmp(exit_env, 2);
	}

	/* wait until at least one of the variables has been written */
	if (!trig_profile(pclt)) {
		printf("Could not trigger on database variables.\n");
		longjmp(exit_env, 3);
	}

	ftime(&start_time);
	/* Loop updates profile every "interval" milliseconds */ 
	while (1) {
		int wrote_it = 0;
		char fnamebuf[80];
		if (item_count >= numitems && unbounded) {
			sprintf(fnamebuf, "%s%d", filename, wrote_it);
			write_profile(fnamebuf, item_count, profile);
			item_count = 0;
		} else if (item_count >= numitems && !unbounded)
			break;

		update_profile_item(pclt, &profile[item_count], &start_time);

		item_count++;
		TIMER_WAIT(ptimer);
	}
	longjmp(exit_env,4);	
}
