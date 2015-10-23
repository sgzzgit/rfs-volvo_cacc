/**\file
 * Database variable update time test: test_update
 *
 * (C) Copyright University of California 2006.  All rights reserved.
 *
 * This program writes a variable to the database at a specified
 * interval, changing the value each cycle. For timing tests.
 *
 */

#include	<sys_os.h>
#include	"db_clt.h"
#include 	"sys_rt.h"
#include	"timestamp.h"

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
typedef struct {
	int new_int;
	double new_double;
} new_type;

int main (int argc, char** argv)
{
	char *domain=DEFAULT_SERVICE;
	char hostname[MAXHOSTNAMELEN+1];
	db_clt_typ *pclt;
	new_type value;
	int opt;
	int var = 50;
	posix_timer_typ *ptmr;
	int millisec = 1000;	/* 1 second interval */
	int numwrites = 60;	/* run for 60 seconds */
	int i;
	int chid;
	int xport = COMM_OS_XPORT;

	value.new_int = 1;
	value.new_double = 2;
	while ((opt = getopt(argc, argv, "d:f:i:n:t:v:")) != -1)
	{
		switch (opt)
		{
		  case 'd':
			domain = strdup(optarg);
			break;
		  case 'i':
			value.new_int = atoi(optarg);	
			break;
		  case 'f':
			value.new_double = atof(optarg);	
			break;
		  case 'n':
			numwrites = atoi(optarg);	
			break;
		  case 't':
			millisec = atoi(optarg);	
			break;
		  case 'v':
			var = atoi(optarg);	
			break;
		  case 'x':
			xport = atoi(optarg);	
			break;
		  default:
			printf( "Usage: %s -d [domain] -v [var] ",argv[0]);
			printf(" -i [int] -f [float] -x [xport] \n");
			exit(EXIT_FAILURE);
		}
	}
	
	chid = ChannelCreate(0);
	if ((ptmr = timer_init(millisec, chid)) == NULL) {
		printf("timer_init failed\n");
		exit(EXIT_FAILURE);
	}

	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = clt_login(argv[0], hostname, domain, xport))
			== NULL)
	{
		printf("clt login %s %s %s %d failed\n",
			argv[0], hostname, domain, xport);
		exit(EXIT_FAILURE);
	}

	if (!clt_create(pclt, var, var, sizeof(new_type))) {
		printf("clt create failed\n");
		clt_logout(pclt);
		exit(EXIT_FAILURE);
	}

	
	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if( setjmp( exit_env ) != 0 )
	{
	    /* Log out from the database. */
	    if (pclt != NULL)
		clt_destroy( pclt, var, var); 
	        clt_logout( pclt );

		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	for (i = 0; i < numwrites || numwrites == 0; i++) {
		if (!clt_update(pclt, var, var, sizeof(value), &value))
			printf("clt update failed\n");
		value.new_int++;
		value.new_double = get_sec_clock();
		TIMER_WAIT(ptmr);
	}
	longjmp( exit_env, 1 );
}

