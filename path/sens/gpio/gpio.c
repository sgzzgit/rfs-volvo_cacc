/* \file 
 *
 *	Requires DB_DIG_IN_VAR to be created externally	
 *
 *	  07-30-2008 JAS
 *	  Modified for use with optical sensors
 *
 * Copyright (c) 2000-7   Regents of the University of California
 *
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.  Upon termination log out of the database.
 *
 */

#include <sys_os.h>
#include <sys_rt.h>
#include "db_clt.h"
#include "db_utils.h"
#include <dscud.h>
#include <timestamp.h>
#include "gpio.h"

jmp_buf exit_env;

#define ERROR_PREFIX "QMM Driver ERROR:"

static void sig_hand( int sig);

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

int main( int argc, char *argv[] )
{
	DSCCB dsccb;	// structure containing board settings
	DSCB dscb;	  // handle used to refer to the board
	ERRPARAMS errorParams;  // structure for returning error 
				// code and error string
	unsigned char digin;
	unsigned char digout;
	short word = -1;
	db_clt_typ *pclt = NULL;		  /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	int opt;
	int output_mask = 0;
	char *domain = DEFAULT_SERVICE; //for QNX6, use "ids"
	int xport = COMM_PSX_XPORT;	//for QNX6, no-op
	posix_timer_typ *ptimer;
	int loop_time = 50;
	char verbose = 0;

	while ((opt = getopt(argc, argv, "d:o:t:c:hv")) != -1) {
		switch (opt) {
			case 'd':
				 domain = strdup(optarg);
				 break;
			case 'o':
				 output_mask = atoi(optarg);
				 break;
			case 't':
				 loop_time = atoi(optarg);
				 break;
			case 'c':
				 word = atoi(optarg) & 0xFF;
				 break;
			case 'v':
				 verbose = 1;
				 break;
			case 'h':
			default:
				 printf("Usage: %s -d [domain]\n\t\t-o ",argv[0]);
				 printf("<output mask: 1=verbose, 2=use DB>");
				 printf("\n\t\t-t loop time, default=50 ms");
				 printf("\n\t\t-c <hw output char>");
				 printf("\n\t\t-v <verbose>\n\t\t");
				 printf("Note: output char trumps db value\n");
				 exit(1);
		}
			}

	/* Log in to the database (shared global memory).  Default to the
	 * the current host. */
	if( output_mask & USE_DATABASE ) {
		get_local_name(hostname, MAXHOSTNAMELEN);
		if (( pclt = db_list_init( argv[0], hostname, domain, xport, 
				 NULL, 0, NULL, 0)) == NULL ) {
			printf("Database initialization error in %s.\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if ((ptimer = timer_init( loop_time, ChannelCreate(0) )) == NULL) {
			fprintf(stderr, "Unable to initialize delay timer\n");
			exit(EXIT_FAILURE);
		}

	if( dscInit( DSC_VERSION ) != DE_NONE ) {
		dscGetLastError(&errorParams);
		fprintf( stderr, "%s %s\n", ERROR_PREFIX, 
					 errorParams.errstring );
		getchar();
		return 0;
	}
	dsccb.base_address = 0x300;
	dsccb.int_level = 5;
	if(dscInitBoard(DSC_QMM, &dsccb, &dscb)!= DE_NONE) {
		dscGetLastError(&errorParams);
		printf("%s %s\n", ERROR_PREFIX, errorParams.errstring);
		return 0;
	}

	if ( setjmp(exit_env) != 0) {
printf("GPIO: Exiting due to signal pclt %d\n",(int)pclt);

		/* Log out from the database. */
		if (pclt != NULL)
		    db_list_done( pclt, NULL, 0, NULL, 0 );
	if (dscFreeBoard(dscb) != DE_NONE) {
		dscGetLastError(&errorParams);
		fprintf(stderr, "%s %s\n", ERROR_PREFIX, errorParams.errstring);
		getchar();
		return 0;
	}
	if (dscFree() != DE_NONE) {
		dscGetLastError(&errorParams);
		fprintf(stderr, "%s %s\n", ERROR_PREFIX, errorParams.errstring);
		getchar();
		return 0;
	}
		exit( EXIT_SUCCESS);
	}
	else 
		sig_ign( sig_list, sig_hand);

	while(1) {
		gpio_dig_in_t gpio_signal_in;

		get_current_timestamp(&gpio_signal_in.ts);

		if( (dscDIOInputByte(dscb, 0, &digin)) != DE_NONE) {
			dscGetLastError(&errorParams);
			fprintf( stderr, "dscDIOInputByte error: %s %s\n", 
			dscGetErrorString(errorParams.ErrCode), 
				errorParams.errstring );
			return 0;
		}

		gpio_signal_in.dig_in = digin;

 		if(verbose){
//			print_timestamp(stdout, &gpio_signal_in.ts);
//			printf(" ");
//			printf("digital in %#0x \n",(unsigned char)(digin & 0xFF));
			print_gpio_dig_in(gpio_signal_in);
			fflush(stdout);
		}

		if( output_mask & USE_DATABASE ) {
			db_clt_read( pclt, DB_DIG_OUT_VAR, sizeof(char), &digout);
			db_clt_write( pclt, DB_DIG_IN_VAR, sizeof(gpio_dig_in_t), &gpio_signal_in);
		}
		if( word >= 0) digout = (unsigned char)word;
		if( (dscDIOOutputByte(dscb, 0, digout)) != DE_NONE) {
			dscGetLastError(&errorParams);
			fprintf(stderr,"dscDIOOutputByte error: %s %s\n",
			dscGetErrorString(errorParams.ErrCode), 
				errorParams.errstring );
			return 0;
		}


		
		TIMER_WAIT( ptimer );
	}
	if (dscFreeBoard(dscb) != DE_NONE) {
		dscGetLastError(&errorParams);
		fprintf(stderr, "%s %s\n", ERROR_PREFIX, errorParams.errstring);
		getchar();
		return 0;
	}
	if (dscFree() != DE_NONE) {
		dscGetLastError(&errorParams);
		fprintf(stderr, "%s %s\n", ERROR_PREFIX, errorParams.errstring);
		getchar();
		return 0;
	}

	return 0;
}


static void sig_hand( int code) {
	if (code == SIGALRM)
		return;
	else
		longjmp( exit_env, code );
}
