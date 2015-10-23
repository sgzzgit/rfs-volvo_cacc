/* FILE: wrlidar.c  Writes a message to the Denso Lidar
 *
 *
 * Copyright (c) 2009   Regents of the University of California
 *
 *  Process to communicate with the Denso Lidar.  This process
 *  writes a message to the Denso lidar.
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.  Upon termination log out of the database.
 *
 */

#include <sys_os.h>
#include <sys_list.h>
#include "sys_rt.h"
#include "db_clt.h"
#include "db_utils.h"
#include "timestamp.h"
#include "densolidar.h"                                           
#include <jbus_vars.h>
#include <jbus.h>

jmp_buf env;

static void sig_hand( int sig);

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR,
};

int status;

void main( int argc, char *argv[] )
{
	mess_union2_typ writeBuff2;
	char hostname[MAXHOSTNAMELEN+1];
	db_clt_typ *pclt;
	db_data_typ db_data;
	j1939_ccvs_typ *pj1939_ccvs;
	float wspeed;
	unsigned char csum;
	int i;
	int option;

	posix_timer_typ *ptimer;       /* Timing proxy */

printf("no args %d\n", argc);

        while ((option = getopt(argc, argv, "h")) != EOF) {
                switch(option) {
                        case 'h':
                        default:
                                printf("Usage: %s >/dev/serX 2>\"logfile\"\n",argv[0]);
                                exit(EXIT_FAILURE);
                                break;
                        }
                }

	/* Setup a timer for every 100 msec. */
	if ((ptimer = timer_init( 100, ChannelCreate(0) )) == NULL)
	    {
	    printf("Unable to initialize wrlidar timer\n");
	    return;
	    }

	/* Log in to the database (shared global memory).  Default to the
	 * the current host. */
         get_local_name(hostname, MAXHOSTNAMELEN);
         if ((pclt = db_list_init(argv[0], hostname, DEFAULT_SERVICE, 
              COMM_OS_XPORT, NULL, 0, NULL, 0)) == NULL) {
	    printf("Database initialization error in wrlidar\n");
	    return;
	    }

	/*	set jmp */
	if ( setjmp(env) != 0)
	    {
	    /* Log out from the database. */
	    if (pclt != NULL)
	        clt_logout( pclt );

		exit( EXIT_SUCCESS);
	}
	else 
		sig_ign( sig_list, sig_hand);

	for (;;)
	    {

	    /* Read the database variable with the wheel speed. */
	    if ( clt_read( pclt, DB_J1939_CCVS_VAR, DB_J1939_CCVS_TYPE,
	                   &db_data ) == FALSE )
	        fprintf( stderr, "clt_read( DB_J1939_CCVS_VAR )\n");
	    pj1939_ccvs = (j1939_ccvs_typ *) db_data.value.user;

	    /* Convert the speed from m/sec to km/hour. */
	    wspeed = pj1939_ccvs->wheel_based_vehicle_speed * 3.6;

	    /* Now format message for a second generation lidar. */
	    writeBuff2.tolidar2_mess.start1 = 0xff;
	    writeBuff2.tolidar2_mess.start2 = 0xff;
	    writeBuff2.tolidar2_mess.function = 0xfd;
	    writeBuff2.tolidar2_mess.cruise_stat = 0x00;
	    writeBuff2.tolidar2_mess.velocity = wspeed;
	    writeBuff2.tolidar2_mess.curve_h = 0x3f; /* Dummy to max curvature */
	    writeBuff2.tolidar2_mess.curve_l = 0xfe; /* dummy to max curvature */
	    writeBuff2.tolidar2_mess.dummy = 0x00;

	    csum = 0;
	    for( i=0; i< 8; i++)
		    csum += writeBuff2.gen_mess2.data[i];

	    if ( csum != 0xff )
	        writeBuff2.tolidar2_mess.checksum = csum;
	    else
	        writeBuff2.tolidar2_mess.checksum = 0x00;

	    /* write the message to the hardware. */
	    write ( STDOUT_FILENO, &writeBuff2, 9 );

	    /* Now wait for proxy from timer (waits 100 msec). */
	    TIMER_WAIT( ptimer );

	    }

}

static void sig_hand( int sig)
{
	longjmp( env, 1);
}
