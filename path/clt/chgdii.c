/**\file 
 *
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *  Process to change the current values stored in the database for 
 *  testing. Operates on the DII_OUT variable. but gives it values from
 *  0 to 15 that are not actually reasonable for the IDS use of this variable.
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.
 *
 */

#include 	<sys_os.h>
#include	"sys_list.h"
#include	"sys_rt.h"
#include	 "db_clt.h"
#include	 "ids_io.h"
#include	 "clt_vars.h"

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
	ERROR
};
static jmp_buf exit_env;
static void sig_hand( int code )
{
	if (code == SIGALRM)
		return;
	else
		longjmp( exit_env, code );
}

int main( int argc, char *argv[] )
{
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	char *domain = DEFAULT_SERVICE;
	dii_out_typ dii_out;
	int value;
	int opt;
	int xport = COMM_OS_XPORT;	// Correct value must be in sys_os.h 

	while ((opt = getopt(argc, argv, "d:x:")) != -1)
	{
		switch (opt)
		{
		  case 'd':
			domain = optarg;
			if (strlen(domain) > 15)
				domain[15] = '\0';
			break;
		  case 'x':
			xport = atoi(optarg);	// for QNX4 or other option
			break;
		  default:
			printf("Usage: chgdii -d [domain] -x [xport] \n");
			exit(1);
		}
	}

	/* Log in to the database (shared global memory).  Default to the
	 * the current host. */
	get_local_name(hostname, MAXHOSTNAMELEN);
	if (( pclt = clt_login( argv[0], hostname, domain, xport)) == NULL )
	    {
	    printf("Database initialization error in chgdii.\n");
	    exit (EXIT_FAILURE);
	    }
	printf("pclt 0x%x, hostname %s\n", (int) pclt, hostname);

	if (clt_create( pclt, DB_DII_OUT_VAR, DB_DII_OUT_TYPE,
		   sizeof( dii_out_typ ) ) == FALSE )
	    {
	    printf("clt_create failed: %d\n", DB_DII_OUT_VAR);
	    exit (EXIT_FAILURE);
	    }
	printf("clt_create %d succeeded\n", DB_DII_OUT_VAR);

	/* Lower priority of this process to 9, since this is not a
	 * critical task. */
	if ( setprio( 0, 9) == ERROR )
	    {
	    fprintf( stderr, "Can't change priority of chgdii to 9.\n" );
	    exit ( EXIT_FAILURE );
	    }

	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if( setjmp( exit_env ) != 0 )
	{
	    /* Log out from the database. */
	    if (pclt != NULL)
		clt_destroy( pclt, DB_DII_OUT_VAR, DB_DII_OUT_VAR); 
	        clt_logout( pclt );

		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

    for(;;)
    {
	    printf("Please enter DII value (0 - 15 ) :");
	    scanf("%d", &value);
	    if( (value < 0) || (value > 15) )
	        {
	        printf("\a\aOnly 0 through 15 accepted!\n");
	        }
	    else
	        {
	        dii_out.dii_flag = value;

	        /* Write updated value to the database. */
	        if (clt_update( pclt, DB_DII_OUT_VAR, DB_DII_OUT_TYPE,
	           sizeof( dii_out_typ ), (void *) &dii_out) == FALSE)
	           fprintf( stderr, "clt_update( DB_DII_OUT_VAR ) failed.\n");
	        }
    }     
}


