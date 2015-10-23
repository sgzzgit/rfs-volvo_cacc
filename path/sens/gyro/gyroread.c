/*
 *
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 *	static char rcsid[] = "$Id: gyroread.c,v 1.1.1.1 2003/04/03 21:14:16 dickey Exp $";
 *
 *
 *	$Log: gyroread.c,v $
 *	Revision 1.1.1.1  2003/04/03 21:14:16  dickey
 *	from D1 60-foot bus
 *	
 *
 *
 *  Process to read KVH E-Core 2000 Fiber Optic Gyro data messages, parse
 *  that information, and put the values into the database.
 *
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.  Upon termination log out of the database.
 *
 */

#include <db_include.h>
#include <db_utils.h>
#include "gyro.h"

#define RATE_CONVERSION 0.00183
/* Converts raw integer value from KVH E-Core 2000 FOG gyro to a
 * floating point value in degrees/second.  Assumes unit is a 60
 * degree/second max rate unit at 100 Hz.  Use 0.000915 for a 30
 * degree/second max rate unit at 100 Hz */

int fpin;	
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
db_clt_typ *pclt;              /* Database client pointer */

#define MAX_MSG_LENGTH         128
#define SERIAL_DEVICE_NAME     "/dev/ttyS6"



int main( int argc, char *argv[] )
{
	unsigned char gyro_msg[8];
	unsigned char msg_minus_sync[7];
	unsigned char checksum;
	int i;
	short signed int temp;
	char hostname[MAXHOSTNAMELEN+1];
	gyro_typ gyro;
	char *domain = DEFAULT_SERVICE;
	int xport = COMM_OS_XPORT;
	int option;
	int noport = 1;
	int nodb = 0;
	char *port = SERIAL_DEVICE_NAME;
	int verbose = 0;
	int veryverbose = 0;
	int mssg_count = 0;
	int err_count = 0;
	/* Read and interpret any user switches. */
	while ( (option = getopt( argc, argv, "nwvp:" )) != EOF )
	    {
	    switch( option )
	        {
	        case 'p':
		    noport = 0;
	            if ( (port = strdup(optarg) ) == 0 )
	                {
	                printf("Bad port address\n");
	            	printf("Usage: %s -v verbose -w very verbose -n no DB -p <serial port>\n", argv[0]);
	                exit( EXIT_FAILURE );
	                }
	            break;
		case 'v':
		    verbose = 1;
                    break;

		case 'w':
		    veryverbose = 1;
		    break;

		case 'n':
		    nodb = 1;
		    break;

	        default:
	            printf("Usage: %s -v verbose -w very verbose -n no DB -p <serial port>\n", argv[0]);
	            exit( EXIT_FAILURE );
	        }
	    }

	if (! nodb) {

	/* Log in to the database. */
	get_local_name(hostname, MAXHOSTNAMELEN);
	if( (pclt = db_list_init( argv[0], hostname, domain, xport,
	         NULL, 0, NULL, 0)) == NULL )
	    {
	    printf("Database initialization error in wrfiledc\n");
            exit( EXIT_FAILURE );
	    }
	}
	/* Initialize serial port. */  
	if (! noport) {
  	    fpin = open( port,  O_RDONLY );
	    if ( fpin <= 0 ) {
		printf( "Error opening device %s for input, gyroread stopped.\n", port);
		exit( EXIT_FAILURE );
	    }

	} else {
	    fpin = STDIN_FILENO;
	    printf( "Using stdin as input\n");
	}

	/*	set jmp */
	if ( setjmp(env) != 0)
	    {

	    /* Log out from the database. */
	    if (! nodb) {
	        if (pclt != NULL)
		    clt_logout( pclt );
	    }
	    printf("%d total messages, %d checksum errors\n", 
		   mssg_count, err_count);
	    exit( EXIT_SUCCESS);

	}
	else 
		sig_ign( sig_list, sig_hand);

	while(1)
	    {

	    /* Read until the beginning of a message, indicated by the
	     * SYNCBIT (the MSB) equal to 1. */
	    gyro_msg[0] = 0;
	    while ( (gyro_msg[0] & 0x80) == 0 ) {
	        read( fpin, &gyro_msg[0], 1 );
 		if (veryverbose) {
		    printf("0x%hhx ", gyro_msg[0]);
		}
	    }

	    mssg_count++;

 	    if (veryverbose) {
	        printf("\nMSSG %d: ", mssg_count);
	    }
	    /* Now read the next seven characters, the remainder of the
	     * current message. */
	    for ( i=1; i<8; i++ )
	    {
	        read( fpin, &gyro_msg[i], 1 );
 		if (veryverbose) {
		    printf("0x%hhx ", gyro_msg[i]);
		}
	    }

  	    if (veryverbose) {
		print_clock();
	    }

	    /* Now interpret the message. */
	    /* The first bit of each byte is a synch bit.  Begin by
	     * removing all of these. */
	    msg_minus_sync[6] =  (gyro_msg[7] & 0x7f) |
	                        ((gyro_msg[6] & 0x01) << 7);
	    msg_minus_sync[5] = ((gyro_msg[6] & 0x7e) >> 1) |
	                        ((gyro_msg[5] & 0x03) << 6);
	    msg_minus_sync[4] = ((gyro_msg[5] & 0x7c) >> 2) |
	                        ((gyro_msg[4] & 0x07) << 5);
	    msg_minus_sync[3] = ((gyro_msg[4] & 0x78) >> 3) |
	                        ((gyro_msg[3] & 0x0f) << 4);
	    msg_minus_sync[2] = ((gyro_msg[3] & 0x70) >> 4) |
	                        ((gyro_msg[2] & 0x1f) << 3);
	    msg_minus_sync[1] = ((gyro_msg[2] & 0x06) >> 5) |
	                        ((gyro_msg[1] & 0x3f) << 2);
	    msg_minus_sync[0] = ((gyro_msg[1] & 0x40) >> 6) |
	                        ((gyro_msg[0] & 0x7f) << 1);

	    /* Verify the checksum. */
	    checksum = 0;
	    for ( i=1; i<7; i++ )
	        checksum = checksum + msg_minus_sync[i];
	    checksum = -checksum;
	    if ( checksum != msg_minus_sync[0] )
	        {
		err_count++;
	        if ( (veryverbose || verbose) && ((err_count % 1000) == 0) )
		    {
		    printf("Checksum error %d reading gyro 0x%hhx vs 0x%hhx (mssg %d error rate %.2f%%)\n", 
			err_count, checksum, msg_minus_sync[0], mssg_count,
			100 * ((float)err_count/(float)mssg_count));
		    }
		gyro.gyro_rate = 99.0;
		if (! nodb) 
		    {
		    if( clt_update( pclt, DB_GYRO_VAR, DB_GYRO_TYPE,
				    sizeof( gyro_typ ), (void *) &gyro ) == FALSE )
		      fprintf( stderr, "clt_update( DB_GYRO_VAR )\n" );
	            }
	        }
	    else
	        {
		
	        temp = (msg_minus_sync[5] << 8) + msg_minus_sync[6];
	        gyro.gyro_rate = temp * RATE_CONVERSION;

	        if(veryverbose || verbose) 
		    {
		    printf("gyro %f (mssg %d)\n", gyro.gyro_rate, mssg_count );
		    }
		if (! nodb) 
		    {
		    if( clt_update( pclt, DB_GYRO_VAR, DB_GYRO_TYPE,
				    sizeof( gyro_typ ), (void *) &gyro ) == FALSE )
		      fprintf( stderr, "clt_update( DB_GYRO_VAR )\n" );
	            }
		}

  	    if (veryverbose) 
	        {
	        printf("DROSS: ");
		}

	    }

}


static void sig_hand( int sig)
{
	longjmp( env, 1);
}
