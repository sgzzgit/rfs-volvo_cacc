/**\file
 * Database variable reader time test: test_read
 *
 * This program reads a variable from a database and displays it on
 * the standard output at "millisec" intervals.
 */

#include	<sys_os.h>
#include	"db_clt.h"
#include	"sys_rt.h"

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
	db_data_typ db_data;
	new_type *presult;
	int opt;
	int var = 50;
	posix_timer_typ *ptmr;
	int millisec = 1000;
	int numreads = 60;
	int i;
	int chid;
	int xport = COMM_OS_XPORT;
	int use_timer = 0;

	while ((opt = getopt(argc, argv, "d:n:t:v:x:")) != -1)
	{
		switch (opt)
		{
		  case 'd':
			domain = strdup(optarg);
			break;
		  case 'n':
			numreads = atoi(optarg);
			break;
		  case 't':
			use_timer = 1;
			millisec = atoi(optarg);
			break;
		  case 'v':
			var = atoi(optarg);
			break;
		  case 'x':
			xport = atoi(optarg);
			break;
		  default:
			printf("Usage: clt_read -d [domain]\n");
			exit(1);
		}
	}
	chid = ChannelCreate(0);
	printf("chid %d\n", chid);

	if ((ptmr = timer_init(millisec, chid)) == NULL) {
		printf("timer_init failed\n");
		exit(1);
	}
	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = clt_login(argv[0], hostname, domain, xport))
			== NULL)
	{
		printf("clt login %s %s %s %d failed\n",
			argv[0], hostname, domain, xport);
		exit(EXIT_FAILURE);
	}

	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if( setjmp( exit_env ) != 0 )
	{
	    /* Log out from the database. */
	    if (pclt != NULL)
	        clt_logout( pclt );

		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	for (i = 0; i < numreads; i++) {	
		if (!clt_read(pclt, var, var, &db_data))
			printf("clt read failed\n");
		else {
			presult = (new_type *) &db_data.value.user[0];
			printf("%d %.3f\n", presult->new_int,
			presult->new_double);
		}
		TIMER_WAIT(ptmr);
	}
	longjmp(exit_env, 1);
}

