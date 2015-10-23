/**\file
 * 
 * rcv_io.c    Process to test IP_Receive loop with both trigger and timer 
 *
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *
 */

#include	<sys_os.h>
#include        "db_clt.h"
#include	"sys_rt.h"
#include	"clt_vars.h"
#include	"ids_io.h"

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

static db_clt_typ *database_init(char *pclient, char *phost, char *pserv,
			int xport );
static void veh_done( db_clt_typ *pclt );

static void sig_hand( int code );

int main( int argc, char *argv[] )
{
	db_clt_typ *pclt;
	char hostname[MAXHOSTNAMELEN+1];
	db_data_typ db_data;
	posix_timer_typ *ptmr;
	dii_out_typ *pdii_out;
	int recv_type;
	int millisec = 5000;
	trig_info_typ trig_info;

	pclt = NULL;

	/* Initialize the database. */
	get_local_name(hostname, MAXHOSTNAMELEN);
	if( (pclt = database_init(argv[0], hostname, DEFAULT_SERVICE,
			COMM_QNX6_XPORT )) == NULL )
	    {
	    fprintf(stderr, "Database initialization error in ids_io\n");
	    veh_done( pclt );
	    exit( EXIT_FAILURE );
	    }

	/* Initialize the timer. */
        if ((ptmr = timer_init(millisec, DB_CHANNEL(pclt))) == NULL)
	    {
            printf("timer_init failed\n");
            exit( EXIT_FAILURE );
            }
	    
	print_timer(ptmr);
	if( setjmp( exit_env ) != 0 )
	{
	    veh_done( pclt );
		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	for( ;; )
	{
	    /* Now wait for a trigger. */
	    recv_type= clt_ipc_receive(pclt, &trig_info, sizeof(trig_info));

	    if (recv_type == DB_TIMER)
		{
		printf("received timer alarm\n");

	        }
	    else if(DB_TRIG_VAR(&trig_info) ==  DB_DII_OUT_VAR )
	        {
		fflush(stdout);
	        /* Read DB_DII_OUT_VAR and send DII control
	         * to the hardware. */
	        if( clt_read( pclt, DB_DII_OUT_VAR, DB_DII_OUT_TYPE,
	            &db_data ) == FALSE )
	            {
	            fprintf( stderr, "clt_read( DB_DII_OUT_VAR ).\n" );
	            }
	        else
	            {
	            pdii_out = (dii_out_typ *) db_data.value.user;
		    printf("dii_out flag change %d\n", pdii_out->dii_flag);
	            }
	        }
		
	    else
		printf("Unknown trigger, recv_type %d\n", recv_type);

        }
}


static db_clt_typ *database_init(char *pclient, char *phost, char *pserv,
					int xport )
{
	db_clt_typ *pclt;

	if( (pclt = clt_login( pclient, phost, pserv, xport)) == NULL )
	{
	    return( NULL );
	}
   	else if (clt_trig_set( pclt, DB_DII_OUT_VAR, DB_DII_OUT_TYPE )
			 == FALSE ) 
	{
	    clt_logout( pclt );
	    return( NULL );
	}
	else
	    return( pclt );
}


static void veh_done( db_clt_typ *pclt)
{

	if( pclt != NULL )
	    {
	    clt_trig_unset( pclt, DB_DII_OUT_VAR, DB_DII_OUT_TYPE );
	    clt_logout( pclt );
	    }

}
