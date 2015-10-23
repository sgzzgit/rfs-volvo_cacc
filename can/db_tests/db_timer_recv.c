#include <stdint.h>
#include	<sys_os.h>
#include        "db_clt.h"
#include	"sys_rt.h"
#include <sys/neutrino.h>
#include <sys/syspage.h>

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
    db_clt_typ *pclt = NULL;
    char hostname[MAXHOSTNAMELEN+1];
    db_data_typ db_data;
    posix_timer_typ *ptmr;
    int recv_type;
    int millisec = 5000;
    trig_info_typ trig_info;
    uint64_t ticksPerMilliSec = SYSPAGE_ENTRY(qtime)->cycles_per_sec / 1000000;
    unsigned count = 0;
    unsigned total_diff = 0;

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
	printf("average timediff = %u\n", total_diff / count);
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
	else if(DB_TRIG_VAR(&trig_info) ==  200)
	{
	    fflush(stdout);
	    /* Read DB_DII_OUT_VAR and send DII control
	     * to the hardware. */
	    if( clt_read( pclt, 200, 200, &db_data ) == FALSE)
	    {
		fprintf( stderr, "clt_read( DB_DII_OUT_VAR ).\n" );
	    }

	    else
	    {
		uint64_t *incoming_time = (uint64_t*) db_data.value.user;
		uint64_t timediff = ClockCycles() - *incoming_time;
		
		timediff /= ticksPerMilliSec;
		total_diff += timediff;
		++count;
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
    else if (clt_trig_set( pclt, 200, 200 ) == FALSE)
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
	clt_trig_unset( pclt, 200, 200 );
	clt_logout( pclt );
    }

}
