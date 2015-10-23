#include 	<sys_os.h>
#include	"sys_list.h"
#include	"sys_rt.h"
#include	 "db_clt.h"
#include <sys/neutrino.h>

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
    int opt;
    int xport = COMM_OS_XPORT;	// Correct value must be in sys_os.h

    int nmax = 0;
    int millisecs = 1000;
    posix_timer_typ *ptmr;
    uint32_t data[2] = {0, 0};

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
	case 't':
	    millisecs = atoi(optarg);
	    break;
	case 'n':
	    nmax = atoi(optarg);
	    break;
	default:
	    printf("Usage: chgdii -d [domain] -x [xport] \n");
	    exit(1);
	}
    }

    if ((ptmr = timer_init(millisecs, 0)) == NULL)
    {
	printf("timer_init failed in %s.\n", argv[0]);
	exit(EXIT_FAILURE);
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

    if (clt_create( pclt, 200, 200, sizeof(data)) == FALSE)
    {
	printf("clt_create failed: %d\n", 200);
	exit (EXIT_FAILURE);
    }
    printf("clt_create %d succeeded\n", 200);

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
	    clt_destroy( pclt, 200, 200); 
	clt_logout( pclt );

	exit( EXIT_SUCCESS );
    }
    else
	sig_ign( sig_list, sig_hand );

    for(;;)
    {
	/* Write updated value to the database. */
	uint64_t time = ClockCycles();
	if (clt_update(pclt, 200, 200, sizeof(time), &time) == FALSE)
	    fprintf( stderr, "clt_update( 200 ) failed.\n");

	if (data[0] > nmax && nmax != 0)
	    longjmp(exit_env, 2);

//	TIMER_WAIT(ptmr);
    }
}
